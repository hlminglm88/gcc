/* Gimple Represented as Polyhedra.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Contributed by Alexandru Plesco <shurikx@gmail.com>
   and Sebastian Pop <pop@cri.ensmp.fr>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/* This pass converts GIMPLE to GRAPHITE, performs some loop
   transformations and then converts the resulting representation back
   to GIMPLE.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "ggc.h"
#include "tree.h"
#include "rtl.h"
#include "basic-block.h"
#include "diagnostic.h"
#include "tree-flow.h"
#include "tree-dump.h"
#include "timevar.h"
#include "cfgloop.h"
#include "tree-chrec.h"
#include "tree-data-ref.h"
#include "tree-scalar-evolution.h"
#include "tree-pass.h"
#include "graphite.h"


/* Print SCOP to FILE.  */

static void
print_scop (FILE *file, scop_p scop)
{
  basic_block bb;
  fprintf (file, "\nSCoP (\n");

  FOR_BB_BETWEEN (bb, scop->entry, scop->exit, next_bb)
    {
      fprintf (file, "bb_%d \n", bb->index);
      fprintf (file, "{\n");
      tree_dump_bb (bb, file, 2);
      fprintf (file, "}\n");
    }

  fprintf (file, ")\n");
}


/* Debug SCOP.  */

static void
debug_scop (scop_p scop)
{
  print_scop (stderr, scop);
}


/* Return true when EXPR is an affine function in LOOP.  */

static bool
affine_expr(struct loop *loop, tree expr)
{
  tree scev = analyze_scalar_evolution (loop, expr);

  scev = instantiate_parameters (loop, scev);

  return evolution_function_is_affine_multivariate_p (scev);
}


/* Return true only when STMT is simple enough for being handled by
   GRAPHITE.  */

static bool
stmt_simple_for_scop_p (tree stmt)
{
  struct loop *loop = bb_for_stmt (stmt)->loop_father;

  /* ASM_EXPR and CALL_EXPR may embed arbitrary side effects.
     Calls have side-effects, except those to const or pure
     functions.  */
  if ((TREE_CODE (stmt) == CALL_EXPR
       && !(call_expr_flags (stmt) & (ECF_CONST | ECF_PURE)))
      || (TREE_CODE (stmt) == ASM_EXPR
	  && ASM_VOLATILE_P (stmt)))
    return false;

  switch (TREE_CODE (stmt))
    {
    case LABEL_EXPR:
      return true;

    case COND_EXPR:
      {
	tree opnd0 = TREE_OPERAND (stmt, 0);

	switch (TREE_CODE (opnd0))
	  {
	  case NE_EXPR:
	  case EQ_EXPR:
	  case LT_EXPR:
	  case GT_EXPR:
	  case LE_EXPR:
	  case GE_EXPR:
	    return (affine_expr(loop, TREE_OPERAND (opnd0, 0)) 
		    && affine_expr(loop, TREE_OPERAND (opnd0, 1)));
	  default:
	    return false;
	  }
      }

    case MODIFY_EXPR:
      {
	tree opnd0 = TREE_OPERAND (stmt, 0);
	tree opnd1 = TREE_OPERAND (stmt, 1);
		
	if ((TREE_CODE (opnd0) == ARRAY_REF 
	     || TREE_CODE (opnd0) == INDIRECT_REF
	     || TREE_CODE (opnd0) == COMPONENT_REF)
	    && !create_data_ref (opnd0, stmt, false))
	  return false;

	if ((TREE_CODE (opnd1) == ARRAY_REF 
	     || TREE_CODE (opnd1) == INDIRECT_REF
	     || TREE_CODE (opnd1) == COMPONENT_REF)
	    && !create_data_ref (opnd1, stmt, true))
	  return false;

	return (affine_expr(loop, opnd0) && affine_expr(loop, opnd1));
      }

    case CALL_EXPR:
      {
	tree args;

	for (args = TREE_OPERAND (stmt, 1); args; args = TREE_CHAIN (args))
	  if ((TREE_CODE (TREE_VALUE (args)) == ARRAY_REF
	       || TREE_CODE (TREE_VALUE (args)) == INDIRECT_REF
	       || TREE_CODE (TREE_VALUE (args)) == COMPONENT_REF)
	      && !create_data_ref (TREE_VALUE (args), stmt, true))
	    return false;

	break;
      }

    default:
      return false;
    }

  return false;
}

/* This function verify if a basic block contains simple statements,
   returns true if a BB contains only simple statements.  */

static bool
basic_block_simple_for_scop_p (basic_block bb)
{
  block_stmt_iterator bsi;

  for (bsi = bsi_start (bb); !bsi_end_p (bsi); bsi_next (&bsi))
    if (!stmt_simple_for_scop_p (bsi_stmt (bsi)))
      return false;

  return true;
}

/* Find static control parts (scops) in LOOPS, and save these in the
   SCOPS vector.  */

static void
build_scops (VEC (scop_p, heap) **scops)
{
  unsigned int j;
  basic_block bb;
  bitmap_iterator bi;
  bitmap not_affine = BITMAP_ALLOC (NULL);
  scop_p scoptp;

  /* Mark basic blocks that contain difficult constructs.  These
     blocks are boundaries of scops.  */
  FOR_EACH_BB (bb)
    {
      if (!basic_block_simple_for_scop_p (bb))
	{
	  bitmap_set_bit (not_affine, bb->index);
	  fprintf (stderr, "index = %d|", bb->index);
	}
    }

  /* Loops containing difficult constructs are also split into several
     scops: mark loop headers and loop latches as points that split
     scops.  */
  EXECUTE_IF_SET_IN_BITMAP (not_affine, 0, j, bi)
    {
      basic_block btemp = BASIC_BLOCK (j);
      struct loop *loop_temp = btemp->loop_father;
      while (loop_temp)
	{
	  bitmap_set_bit (not_affine, loop_temp->header->index);
	  bitmap_set_bit (not_affine, loop_temp->latch->index);
	  loop_temp = loop_temp->outer;
	}
    }

  /* Split the CFG into scops.  */
  scoptp = XNEW (struct scop);
  scoptp->entry = BASIC_BLOCK (0);

  EXECUTE_IF_SET_IN_BITMAP (not_affine, 1, j, bi)
    {
      scoptp->exit =  BASIC_BLOCK (j-1);
      VEC_safe_push (scop_p, heap, *scops, scoptp);

      scoptp = XNEW (struct scop);
      scoptp->entry =  BASIC_BLOCK (j);
    }
}

/* Build the domains for each loop in the SCOP.  */

static void
build_domains (scop_p scop ATTRIBUTE_UNUSED)
{
  
}


/* Build a schedule for each statement in the SCOP.  */

static void
build_scattering_functions (scop_p scop ATTRIBUTE_UNUSED)
{
  
}


/* Transform the SCOP.  */

static void
graphite_transform (scop_p scop ATTRIBUTE_UNUSED)
{
  
}


/* GIMPLE Loop Generator: generates loops in GIMPLE form for SCOP.  */

static void
gloog (scop_p scop ATTRIBUTE_UNUSED)
{
  
}


/* Perform a set of linear transforms on LOOPS.  */

void
graphite_transform_loops (struct loops *loops ATTRIBUTE_UNUSED)
{
  unsigned i;
  scop_p scop;
  VEC (scop_p, heap) *scops = VEC_alloc (scop_p, heap, 3);

  if (dump_file && (dump_flags & TDF_DETAILS))
    fprintf (dump_file, "Graphite loop transformations \n");

  build_scops (&scops);

  for (i = 0; VEC_iterate (scop_p, scops, i, scop); i++)
    {
      build_domains (scop);
      build_scattering_functions (scop);
      
      debug_scop (scop);

      graphite_transform (scop);
      gloog (scop);
    }
}
