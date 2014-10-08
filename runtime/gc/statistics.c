/* Copyright (C) 1999-2005 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 * Copyright (C) 1997-2000 NEC Research Institute.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 */

void incSync (GC_state s) {
  switch (s->syncReason) {
    case SYNC_NONE:
      fprintf (stderr, "Got to begin without a reason?\n");
      assert (0);
      exit (1);
      break;
    case SYNC_SIGNALS:
      break;
    case SYNC_OLD_GEN_ARRAY:
      s->globalState.cumulativeStatistics->syncForOldGenArray++;
      break;
    case SYNC_NEW_GEN_ARRAY:
      s->globalState.cumulativeStatistics->syncForNewGenArray++;
      break;
    case SYNC_STACK:
      s->globalState.cumulativeStatistics->syncForStack++;
      break;
    case SYNC_HEAP:
      s->globalState.cumulativeStatistics->syncForHeap++;
      break;
    case SYNC_FORCE:
      s->globalState.cumulativeStatistics->syncForce++;
      break;
    case SYNC_LIFT:
      s->globalState.cumulativeStatistics->syncForLift++;
      break;
    case SYNC_PACK:
      s->globalState.cumulativeStatistics->syncMisc++;
      break;
    case SYNC_SAVE_WORLD:
      s->globalState.cumulativeStatistics->syncMisc++;
      break;
    case SYNC_MISC:
      s->globalState.cumulativeStatistics->syncMisc++;
      break;
    case SYNC_LIFT_NO_GC:
      s->globalState.cumulativeStatistics->syncForLiftNoGC++;
      break;
    case SYNC_HELP:
      break;
    default:
      fprintf (stderr, "Unknown sync reason?\n");
      exit (1);
  }
}
