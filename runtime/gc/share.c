/* Copyright (C) 1999-2008 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 * Copyright (C) 1997-2000 NEC Research Institute.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 */

void GC_share (GC_state s, pointer object) {
  size_t bytesExamined;
  size_t bytesHashConsed;

  s->syncReason = SYNC_FORCE;
  //XXX SPH I shouldn't need ENTER/LEAVE here as share will only be
  // invoked on objects I own. But it might traverse shared heap... So??
  ENTER0 (s); /* update stack in heap, in case it is reached */

  if (DEBUG_SHARE)
    fprintf (stderr, "GC_share "FMTPTR" [%d]\n", (uintptr_t)object,
             Proc_processorNumber (s));
  if (DEBUG_SHARE or s->globalState.controls->messages)
    s->lastMajorStatistics->bytesHashConsed = 0;
  // Don't hash cons during the first round of marking.
  bytesExamined =
    dfsMarkByMode (s, object, emptyForeachObjectFun, MARK_MODE,
                   FALSE, FALSE, FALSE, FALSE);
  s->objectHashTable = allocHashTable (s);
  // Hash cons during the second round of (un)marking.
  dfsMarkByMode (s, object, emptyForeachObjectFun, UNMARK_MODE,
                 TRUE, FALSE, FALSE, FALSE);
  freeHashTable (s->objectHashTable);
  bytesHashConsed = s->lastMajorStatistics->bytesHashConsed;
  s->globalState.cumulativeStatistics->bytesHashConsed += bytesHashConsed;
  if (DEBUG_SHARE or s->globalState.controls->messages)
    printBytesHashConsedMessage (bytesHashConsed, bytesExamined);
  LEAVE0 (s);

  size_t res = GC_size (s, object);
  printf ("%ld\n", res);
}
