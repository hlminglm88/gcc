/* OpenACC Runtime initialization routines

   Copyright (C) 2013-2014 Free Software Foundation, Inc.

   Contributed by Mentor Embedded.

   This file is part of the GNU OpenMP Library (libgomp).

   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "openacc.h"
#include "config.h"
#include "libgomp.h"
#include "gomp-constants.h"
#include "target.h"
#include "oacc-int.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "splay-tree.h"

/* Return block containing [H->S), or NULL if not contained.  */

attribute_hidden splay_tree_key
lookup_host (struct gomp_memory_mapping *mem_map, void *h, size_t s)
{
  struct splay_tree_key_s node;
  splay_tree_key key;

  node.host_start = (uintptr_t) h;
  node.host_end = (uintptr_t) h + s;

  gomp_mutex_lock (&mem_map->lock);

  key = splay_tree_lookup (&mem_map->splay_tree, &node);

  gomp_mutex_unlock (&mem_map->lock);

  return key;
}

/* Return block containing [D->S), or NULL if not contained.
   The list isn't ordered by device address, so we have to iterate
   over the whole array.  This is not expected to be a common
   operation.  */

static splay_tree_key
lookup_dev (struct target_mem_desc *tgt, void *d, size_t s)
{
  int i;
  struct target_mem_desc *t;
  struct gomp_memory_mapping *mem_map;
  
  if (!tgt)
    return NULL;
  
  mem_map = tgt->mem_map;

  gomp_mutex_lock (&mem_map->lock);

  for (t = tgt; t != NULL; t = t->prev)
    {
      if (t->tgt_start <= (uintptr_t) d && t->tgt_end >= (uintptr_t) d + s)
        break;
    }

  gomp_mutex_unlock (&mem_map->lock);

  if (!t)
    return NULL;

  for (i = 0; i < t->list_count; i++)
    {
      void * offset;

      splay_tree_key k = &t->array[i].key;
      offset = d - t->tgt_start + k->tgt_offset;

      if (k->host_start + offset <= (void *) k->host_end)
        return k;
    }
 
  return NULL;
}

/* OpenACC is silent on how memory exhaustion is indicated.  We return
   NULL.  */

void *
acc_malloc (size_t s)
{
  if (!s)
    return NULL;

  ACC_lazy_initialize ();

  return base_dev->device_alloc_func (s);
}

/* OpenACC 2.0a (3.2.16) doesn't specify what to do in the event
   the device address is mapped. We choose to check if it mapped,
   and if it is, to unmap it. */
void
acc_free (void *d)
{
  splay_tree_key k;
  struct goacc_thread *thr = goacc_thread ();

  if (!d)
    return;

  /* We don't have to call lazy open here, as the ptr value must have
     been returned by acc_malloc.  It's not permitted to pass NULL in
     (unless you got that null from acc_malloc).  */
  if ((k = lookup_dev (thr->dev->openacc.data_environ, d, 1)))
   {
     void *offset;

     offset = d - k->tgt->tgt_start + k->tgt_offset;

     acc_unmap_data ((void *)(k->host_start + offset));
   }

  base_dev->device_free_func (d);
}

void
acc_memcpy_to_device (void *d, void *h, size_t s)
{
  /* No need to call lazy open here, as the device pointer must have
     been obtained from a routine that did that.  */
  base_dev->device_host2dev_func (d, h, s);
}

void
acc_memcpy_from_device (void *h, void *d, size_t s)
{
  /* No need to call lazy open here, as the device pointer must have
     been obtained from a routine that did that.  */
  base_dev->device_dev2host_func (h, d, s);
}

/* Return the device pointer that corresponds to host data H.  Or NULL
   if no mapping.  */

void *
acc_deviceptr (void *h)
{
  splay_tree_key n;
  void *d;
  void *offset;

  ACC_lazy_initialize ();

  struct goacc_thread *thr = goacc_thread ();

  n = lookup_host (&thr->dev->mem_map, h, 1);

  if (!n)
    return NULL;

  offset = h - n->host_start;

  d = n->tgt->tgt_start + n->tgt_offset + offset;

  return d;
}

/* Return the host pointer that corresponds to device data D.  Or NULL
   if no mapping.  */

void *
acc_hostptr (void *d)
{
  splay_tree_key n;
  void *h;
  void *offset;

  ACC_lazy_initialize ();

  struct goacc_thread *thr = goacc_thread ();

  n = lookup_dev (thr->dev->openacc.data_environ, d, 1);

  if (!n)
    return NULL;

  offset = d - n->tgt->tgt_start + n->tgt_offset;

  h = n->host_start + offset;

  return h;
}

/* Return 1 if host data [H,+S] is present on the device.  */

int
acc_is_present (void *h, size_t s)
{
  splay_tree_key n;

  if (!s || !h)
    return 0;

  ACC_lazy_initialize ();

  struct goacc_thread *thr = goacc_thread ();
  struct gomp_device_descr *acc_dev = thr->dev;

  n = lookup_host (&acc_dev->mem_map, h, s);

  if (n && ((uintptr_t)h < n->host_start
	    || (uintptr_t)h + s > n->host_end
	    || s > n->host_end - n->host_start))
    n = NULL;

  return n != NULL;
}

/* Create a mapping for host [H,+S] -> device [D,+S] */

void
acc_map_data (void *h, void *d, size_t s)
{
  struct target_mem_desc *tgt;
  size_t mapnum = 1;
  void *hostaddrs = h;
  void *devaddrs = d;
  size_t sizes = s;
  unsigned short kinds = GOMP_MAP_ALLOC;

  ACC_lazy_initialize ();

  struct goacc_thread *thr = goacc_thread ();
  struct gomp_device_descr *acc_dev = thr->dev;

  if (acc_dev->capabilities & TARGET_CAP_SHARED_MEM)
    {
      if (d != h)
        gomp_fatal ("cannot map data on shared-memory system");

      tgt = gomp_map_vars (NULL, NULL, 0, NULL, NULL, NULL, NULL, true, false);
    }
  else
    {
      struct goacc_thread *thr = goacc_thread ();

      if (!d || !h || !s)
	gomp_fatal ("[%p,+%d]->[%p,+%d] is a bad map",
                    (void *)h, (int)s, (void *)d, (int)s);

      if (lookup_host (&acc_dev->mem_map, h, s))
	gomp_fatal ("host address [%p, +%d] is already mapped", (void *)h,
		    (int)s);

      if (lookup_dev (thr->dev->openacc.data_environ, d, s))
	gomp_fatal ("device address [%p, +%d] is already mapped", (void *)d,
		    (int)s);

      tgt = gomp_map_vars ((struct gomp_device_descr *) acc_dev,
			   &acc_dev->mem_map, mapnum, &hostaddrs,
			   &devaddrs, &sizes, &kinds, true, false);
    }

  tgt->prev = acc_dev->openacc.data_environ;
  acc_dev->openacc.data_environ = tgt;
}

void
acc_unmap_data (void *h)
{
  struct goacc_thread *thr = goacc_thread ();
  struct gomp_device_descr *acc_dev = thr->dev;

  /* No need to call lazy open, as the address must have been mapped.  */

  size_t host_size;
  splay_tree_key n = lookup_host (&acc_dev->mem_map, h, 1);
  struct target_mem_desc *t;

  if (!n)
    gomp_fatal ("%p is not a mapped block", (void *)h);

  host_size = n->host_end - n->host_start;

  if (n->host_start != (uintptr_t) h)
    gomp_fatal ("[%p,%d] surrounds1 %p",
        	(void *) n->host_start, (int) host_size, (void *) h);

  t = n->tgt;

  if (t->refcount == 2)
    {
      struct target_mem_desc *tp;

      /* This is the last reference, so pull the descriptor off the 
         chain. This avoids gomp_unmap_vars via gomp_unmap_tgt from
         freeing the device memory. */
      t->tgt_end = 0;
      t->to_free = 0;

      gomp_mutex_lock (&acc_dev->mem_map.lock);

      for (tp = NULL, t = acc_dev->openacc.data_environ; t != NULL;
	   tp = t, t = t->prev)
        if (n->tgt == t)
          {
            if (tp)
              tp->prev = t->prev;
            else
              acc_dev->openacc.data_environ = t->prev;

            break; 
          }

      gomp_mutex_unlock (&acc_dev->mem_map.lock);
    }
  
  gomp_unmap_vars (t, true);
}

#define PCC_Present (1 << 0)
#define PCC_Create (1 << 1)
#define PCC_Copy (1 << 2)

attribute_hidden void *
present_create_copy (unsigned f, void *h, size_t s)
{
  void *d;
  splay_tree_key n;

  if (!h || !s)
    gomp_fatal ("[%p,+%d] is a bad range", (void *)h, (int)s);

  ACC_lazy_initialize ();

  struct goacc_thread *thr = goacc_thread ();
  struct gomp_device_descr *acc_dev = thr->dev;

  n = lookup_host (&acc_dev->mem_map, h, s);
  if (n)
    {
      /* Present. */
      d = (void *) (n->tgt->tgt_start + n->tgt_offset);

      if (!(f & PCC_Present))
        gomp_fatal ("[%p,+%d] already mapped to [%p,+%d]",
            (void *)h, (int)s, (void *)d, (int)s);
      if ((h + s) > (void *)n->host_end)    
        gomp_fatal ("[%p,+%d] not mapped", (void *)h, (int)s);
    }
  else if (!(f & PCC_Create))
    {
      gomp_fatal ("[%p,+%d] not mapped", (void *)h, (int)s);
    }
  else
    {
      struct target_mem_desc *tgt;
      size_t mapnum = 1;
      unsigned short kinds;
      void *hostaddrs = h;

      if (f & PCC_Copy)
        kinds = GOMP_MAP_ALLOC_TO;
      else
        kinds = GOMP_MAP_ALLOC;

      tgt = gomp_map_vars ((struct gomp_device_descr *) acc_dev,
			   &acc_dev->mem_map, mapnum, &hostaddrs,
			   NULL, &s, &kinds, true, false);

      gomp_mutex_lock (&acc_dev->mem_map.lock);

      d = tgt->to_free;
      tgt->prev = acc_dev->openacc.data_environ;
      acc_dev->openacc.data_environ = tgt;

      gomp_mutex_unlock (&acc_dev->mem_map.lock);
    }
  
  return d;
}

void *
acc_create (void *h, size_t s)
{
  return present_create_copy (PCC_Create, h, s);
}

void *
acc_copyin (void *h, size_t s)
{
  return present_create_copy (PCC_Create | PCC_Copy, h, s);
}

void *
acc_present_or_create (void *h, size_t s)
{
  return present_create_copy (PCC_Present | PCC_Create, h, s);
}

void *
acc_present_or_copyin (void *h, size_t s)
{
  return present_create_copy (PCC_Present | PCC_Create | PCC_Copy, h, s);
}

#define DC_Copyout (1 << 0)

static void
delete_copyout (unsigned f, void *h, size_t s)
{
  size_t host_size;
  splay_tree_key n;
  void *d;
  struct goacc_thread *thr = goacc_thread ();
  struct gomp_device_descr *acc_dev = thr->dev;

  n = lookup_host (&acc_dev->mem_map, h, s);

  /* No need to call lazy open, as the data must already have been
     mapped.  */

  if (!n)
    gomp_fatal ("[%p,%d] is not mapped", (void *)h, (int)s);

  d = (void *) (n->tgt->tgt_start + n->tgt_offset);

  host_size = n->host_end - n->host_start;

  if (n->host_start != (uintptr_t) h || host_size != s)
    gomp_fatal ("[%p,%d] surrounds2 [%p,+%d]",
        	(void *) n->host_start, (int) host_size, (void *) h, (int) s);

  if (f & DC_Copyout)
    acc_dev->device_dev2host_func (h, d, s);
  
  acc_unmap_data (h);

  acc_dev->device_free_func (d);
}

void
acc_delete (void *h , size_t s)
{
  delete_copyout (0, h, s);
}

void acc_copyout (void *h, size_t s)
{
  delete_copyout (DC_Copyout, h, s);
}

static void
update_dev_host (int is_dev, void *h, size_t s)
{
  splay_tree_key n;
  void *d;
  struct goacc_thread *thr = goacc_thread ();
  struct gomp_device_descr *acc_dev = thr->dev;

  n = lookup_host (&acc_dev->mem_map, h, s);

  /* No need to call lazy open, as the data must already have been
     mapped.  */

  if (!n)
    gomp_fatal ("[%p,%d] is not mapped", h, (int)s);

  d = (void *) (n->tgt->tgt_start + n->tgt_offset);

  if (is_dev)
    acc_dev->device_host2dev_func (d, h, s);
  else
    acc_dev->device_dev2host_func (h, d, s);
}

void
acc_update_device (void *h, size_t s)
{
  update_dev_host (1, h, s);
}

void
acc_update_self (void *h, size_t s)
{
  update_dev_host (0, h, s);
}
