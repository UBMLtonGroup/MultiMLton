/* Copyright (C) 1999-2005 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 * Copyright (C) 1997-2000 NEC Research Institute.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 */

#if (defined (MLTON_GC_INTERNAL_FUNCS))

static inline void updateWeaksForCheneyCopy (GC_state s);
static inline void swapHeapsForCheneyCopy (GC_state s);
static inline void swapHeapsForSharedCheneyCopy (GC_state s);
static void majorCheneyCopyGC (GC_state s);
static void majorCheneyCopySharedGC (GC_state s);
static void minorCheneyCopyGC (GC_state s);

#endif /* (defined (MLTON_GC_INTERNAL_FUNCS)) */