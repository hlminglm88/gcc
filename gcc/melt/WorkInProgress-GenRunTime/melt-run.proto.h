/*  -*- C -*-  
  MELT file melt-run.h included in every generated file.
   all include files for generated code
   
   Copyright (C) 2008,2009,2011 Free Software Foundation, Inc.
   Contributed by Basile Starynkevitch <basile@starynkevitch.net>

This file is part of GCC. It is produced from melt-run.proto.h

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.


GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.   If not see
<http://www.gnu.org/licenses/>.    */


/* all file including this are somehow plugins, perhaps MELT specific ones */
#include "gcc-plugin.h"


/* usual GCC middle-end includes, copied from melt-runtime.c */

#include "config.h"

/* this is intended to pass -DDISABLE_CHECKING explicitly */
#ifdef DISABLE_CHECKING
#undef ENABLE_CHECKING
#endif

#include "system.h"
#include "coretypes.h"
#include "obstack.h"
#include "tm.h"
#include "tree.h"
#include "gimple.h"
#include "filenames.h"
#include "tree-pass.h"
#include "tree-dump.h"
#include "tree-flow.h"
#include "tree-iterator.h"
#include "tree-inline.h"
#include "basic-block.h"
#include "cfgloop.h"
#include "timevar.h"
#include "ggc.h"
#include "cgraph.h"
#include "diagnostic.h"
#include "flags.h"
#include "toplev.h"
#include "options.h"
#include "params.h"
#include "real.h"
#include "prefix.h"
#include "md5.h"
#include "cppdefault.h"


/* MELT specific includes */

/* PPL is used by MELT */
#include "ppl_c.h"

/* the melt-runtime.h header includes itself generated meltrunsup.h
   header! */
#include "melt-runtime.h"


#define meltcallcount -1

#define meltfptr  meltfram__.mcfr_varptr
#define meltfnum  meltfram__.mcfr_varnum
#define meltfclos meltfram__.mcfr_clos
#define meltfrout meltfram__.mcfr_clos->rout

/* these are added by the melt-module.mk file & make */
extern const char melt_compiled_timestamp[];
extern const char melt_md5[];

/* this is generated by gengtype */
extern void gt_ggc_mx_melt_un (void*);



/* actually, the generated melt-run.h contains a MD5 string hash, same
   as melt_run_preprocessed_md5 */
#define MELT_RUN_HASHMD5 XX
/* It helps a lot to have an svn id here, since this file is really
   important, and is installed.  */
/* eof $Id: melt-run.proto.h 165691 2010-10-19 10:26:41Z bstarynk $ */
