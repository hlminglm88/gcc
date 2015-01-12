/* Copyright (C) 2014-2015 Free Software Foundation, Inc.

   This file is part of the GNU Offloading and Multi Processing Library
   (libgomp).

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

#ifndef LIBGOMP_TARGET_H
#define LIBGOMP_TARGET_H 1

/* Capabilities of offloading devices.  */
#define GOMP_OFFLOAD_CAP_SHARED_MEM	(1 << 0)
#define GOMP_OFFLOAD_CAP_NATIVE_EXEC	(1 << 1)
#define GOMP_OFFLOAD_CAP_OPENMP_400	(1 << 2)
#define GOMP_OFFLOAD_CAP_OPENACC_200	(1 << 3)

/* Type of offload target device.  Keep in sync with include/gomp-constants.h.  */
enum offload_target_type
{
  OFFLOAD_TARGET_TYPE_HOST = 2,
  OFFLOAD_TARGET_TYPE_HOST_NONSHM = 3,
  OFFLOAD_TARGET_TYPE_NVIDIA_PTX = 5,
  OFFLOAD_TARGET_TYPE_INTEL_MIC = 6
};

/* Auxiliary struct, used for transferring a host-target address range mapping
   from plugin to libgomp.  */
struct mapping_table
{
  uintptr_t host_start;
  uintptr_t host_end;
  uintptr_t tgt_start;
  uintptr_t tgt_end;
};

#endif /* LIBGOMP_TARGET_H */
