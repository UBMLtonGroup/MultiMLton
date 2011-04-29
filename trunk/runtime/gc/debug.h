/* Copyright (C) 1999-2007 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 * Copyright (C) 1997-2000 NEC Research Institute.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 */

#ifndef DEBUG
#define DEBUG FALSE
#endif


enum {
  DEBUG_ARRAY = FALSE,
  DEBUG_CALL_STACK = FALSE,
  DEBUG_CARD_MARKING = FALSE,
  DEBUG_DETAILED = FALSE,
  DEBUG_DFS_MARK = FALSE,
  DEBUG_ENTER_LEAVE = FALSE,
  DEBUG_GENERATIONAL = FALSE,
  DEBUG_INT_INF = FALSE,
  DEBUG_INT_INF_DETAILED = FALSE,
  DEBUG_LWTGC = TRUE,
  DEBUG_MARK_COMPACT = FALSE,
  DEBUG_MEM = FALSE,
  DEBUG_OBJPTR = FALSE,
  DEBUG_PROFILE = FALSE,
  DEBUG_READ_BARRIER = FALSE,
  DEBUG_RESIZING = FALSE,
  DEBUG_SHARE = FALSE,
  DEBUG_SIGNALS = FALSE,
  DEBUG_SIZE = FALSE,
  DEBUG_SOURCES = FALSE,
  DEBUG_SPLICE = TRUE,
  DEBUG_SQ = TRUE,
  DEBUG_STACKS = FALSE,
  DEBUG_THREADS = FALSE,
  DEBUG_TRANSLATE = FALSE,
  DEBUG_WEAK = FALSE,
  DEBUG_WORLD = FALSE,
  FORCE_GENERATIONAL = FALSE,
  FORCE_MARK_COMPACT = FALSE,
  MEASURE_PARASITE_CLOSURE = FALSE,
};
