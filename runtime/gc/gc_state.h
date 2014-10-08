/* Copyright (C) 1999-2008 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 * Copyright (C) 1997-2000 NEC Research Institute.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 */

#if (defined (MLTON_GC_INTERNAL_TYPES))

struct GC_state {
  /* These fields are at the front because they are the most commonly
   * referenced, and having them at smaller offsets may decrease code
   * size and improve cache performance.
   */
  pointer frontier; /* start <= frontier < limit *///local
  pointer limit; /* limit = heap->start + heap->size *///local
  pointer stackTop; /* Top of stack in current thread. *///local
  pointer stackLimit; /* stackBottom + stackSize - maxFrameSize *///local
  pointer localHeapStart;//local
  pointer sharedHeapStart;//local
  pointer sharedHeapEnd;//local
  pointer sessionStart;//Global
  struct GC_generationalMaps generationalMaps; /* generational maps for this heap *///local

  /* ML arrays and queues */
  SchedulerQueue* schedulerQueue;//local
  Lock* schedulerLocks;//local

  objptr* moveOnWBA;//local
  int32_t moveOnWBASize;//local
  int32_t moveOnWBAMaxSize;//local

  PreemptThread* preemptOnWBA;//local
  int32_t preemptOnWBASize;//local
  int32_t preemptOnWBAMaxSize;//local

  objptr* danglingStackList;//local
  int32_t danglingStackListSize;//local
  int32_t danglingStackListMaxSize;//local

  SpawnThread* spawnOnWBA;//local
  int32_t spawnOnWBASize;//local
  int32_t spawnOnWBAMaxSize;//local

  pointer sharedFrontier;//Global
  pointer sharedLimit;//Global
  bool tmpBool;//local
  pointer tmpPointer;//local
  int32_t tmpInt;//local
  size_t exnStack;//local

  /* Alphabetized fields follow. */
  size_t alignment; /* *///local
  bool amInGC; //local
  bool amOriginal; //local
  uint32_t procId; //local
  char **atMLtons; /* Initial @MLton args, processed before command line. *///global
  uint32_t atMLtonsLength; //global
  uint32_t atomicState; //local
  objptr callFromCHandlerThread; /* Handler for exported C calls (in heap). *///global?//global
  struct GC_callStackState callStackState; //local
  bool canMinor; /* TRUE iff there is space for a minor gc. *///local
  struct GC_controls *controls; //global
  struct GC_cumulativeStatistics *cumulativeStatistics; //global
  objptr currentThread; /* Currently executing thread (in heap). *///local

  struct GC_forwardState forwardState; //local
  pointer ffiOpArgsResPtr; //local
  GC_frameLayout frameLayouts; /* Array of frame layouts. *///local?//local
  uint32_t frameLayoutsLength; /* Cardinality of frameLayouts array. *///local
  /* Currently only used to hold raise operands. XXX at least i think so */
  Pointer *globalObjptrNonRoot;//local
  /* Ordinary globals */
  objptr *globals;//local
  uint32_t globalsLength;//local
  bool hashConsDuringGC;//local
  struct GC_heap *heap;//local
  struct GC_intInfInit *intInfInits;//local
  uint32_t intInfInitsLength;//local
  struct GC_lastMajorStatistics *lastMajorStatistics;//local
  struct GC_lastSharedMajorStatistics *lastSharedMajorStatistics;//local
  pointer limitPlusSlop; /* limit + GC_HEAP_LIMIT_SLOP *///local
  pointer sharedLimitPlusSlop;//local
  pointer start; /* Like heap->nursery but per processor.  nursery <= start <= frontier *///local
  pointer sharedStart;//local
  int (*loadGlobals)(FILE *f); /* loads the globals from the file. *///global
  uint32_t magic; /* The magic number for this executable. *///global
  uint32_t maxFrameSize;//global
  bool mutatorMarksCards;//local
  bool selectiveDebug;//global
  /* For PCML */
  pthread_t pthread;//local
  int32_t timeInterval; /* In milliseconds *///local
  bool enableTimer;//local
  /* The maximum amount of concurrency */
  int32_t numberOfProcs;//local
  /* For I/O threads */
  int32_t numIOThreads;//local
  GC_objectHashTable objectHashTable;//local
  GC_objectType objectTypes; /* Array of object types. *///local
  uint32_t objectTypesLength; /* Cardinality of objectTypes array. *///local
  /* States for each processor */
  GC_state procStates;//global
  struct GC_profiling profiling;//local
  GC_frameIndex (*returnAddressToFrameIndex) (GC_returnAddress ra);//local
  uint32_t returnToC;//local
  /* Roots that may be, for example, on the C call stack */
  objptr *roots;//local
  uint32_t rootsLength;//local
  objptr savedThread; /* Result of GC_copyCurrentThread.
                       * Thread interrupted by arrival of signal.
                       *///local
  objptr savedClosure; /* This is used for switching to a new thread *///local
  objptr pacmlThreadId; /* ThreadId of the current pacml thread *///local
  int (*saveGlobals)(FILE *f); /* saves the globals to the file. *///local
  bool saveWorldStatus; /* *///local
  struct GC_heap *secondaryLocalHeap; /* Used for major copying collection. *///local
  struct GC_heap *sharedHeap; /* Used as a uncollected shared heap for testing lwtgc *///global
  struct GC_heap *secondarySharedHeap; /* Used for major copying collection on shared heap *///global
  objptr signalHandlerThread; /* Handler for signals (in heap). *///local
  struct GC_signalsInfo signalsInfo;//local
  struct GC_sourceMaps sourceMaps;//local
  pointer stackBottom; /* Bottom of stack in current thread. *///local
  uintmax_t startTime; /* The time when GC_init or GC_loadWorld was called. *///local
  int32_t copiedSize;//local
  int32_t syncReason;//local
  struct GC_sysvals sysvals;//local
  struct GC_translateState translateState;//local
  struct GC_vectorInit *vectorInits;//local
  uint32_t vectorInitsLength;//local
  UT_array* reachable;//local
  CopyObjectMap* copyObjectMap;//local
  bool copyImmutable;//local
  GC_weak weaks; /* Linked list of (live) weak pointers *///local
  char *worldFile;//local
  UT_array* directCloXferArray; /* Array to store closures directly transferred to this core *///local

  /* DEV variables
   * ------------
   * The following variables are only used for development purposes. The are to
   * be removed/not used for production/benchmarking runs.
   */
  FILE* fp;//local
};

#endif /* (defined (MLTON_GC_INTERNAL_TYPES)) */

#if (defined (MLTON_GC_INTERNAL_FUNCS))

static void displayGCState (GC_state s, FILE *stream);

static inline size_t sizeofGCStateCurrentStackUsed (GC_state s);
static inline void setGCStateCurrentThreadAndStack (GC_state s);
static void setGCStateCurrentSharedHeap (GC_state s,
                                         size_t oldGenBytesRequested,
                                         size_t nurseryBytesRequested,
                                         bool duringInit);
static void setGCStateCurrentLocalHeap (GC_state s,
                                        size_t oldGenBytesRequested,
                                        size_t nurseryBytesRequested);

static GC_state getGCStateFromPointer (GC_state s, pointer p);

#endif /* (defined (MLTON_GC_INTERNAL_FUNCS)) */

#if (defined (MLTON_GC_INTERNAL_BASIS))

PRIVATE void GC_setSelectiveDebug (GC_state *gs, bool b);
PRIVATE bool GC_getAmOriginal (GC_state *gs);
PRIVATE void GC_setAmOriginal (GC_state *gs, bool b);
PRIVATE bool GC_getIsPCML (void);
PRIVATE void GC_setControlsMessages (GC_state *gs, bool b);
PRIVATE void GC_setControlsSummary (GC_state *gs, bool b);
PRIVATE void GC_setControlsRusageMeasureGC (GC_state *gs, bool b);
PRIVATE uintmax_t GC_getCumulativeStatisticsBytesAllocated (GC_state *gs);
PRIVATE uintmax_t GC_getCumulativeStatisticsNumCopyingGCs (GC_state *gs);
PRIVATE uintmax_t GC_getCumulativeStatisticsNumMarkCompactGCs (GC_state *gs);
PRIVATE uintmax_t GC_getCumulativeStatisticsNumMinorGCs (GC_state *gs);
PRIVATE size_t GC_getCumulativeStatisticsMaxBytesLive (GC_state *gs);
PRIVATE void GC_setHashConsDuringGC (GC_state *gs, bool b);
PRIVATE size_t GC_getLastMajorStatisticsBytesLive (GC_state *gs);

PRIVATE pointer GC_getCallFromCHandlerThread (GC_state *gs);
PRIVATE void GC_setCallFromCHandlerThread (GC_state *gs, pointer p);
PRIVATE pointer GC_getCurrentThread (GC_state *gs);
PRIVATE pointer GC_getSavedThread (GC_state *gs);
PRIVATE void GC_setSavedThread (GC_state *gs, pointer p);
PRIVATE void GC_setSignalHandlerThread (GC_state *gs, pointer p);

PRIVATE void GC_print (int);
PRIVATE inline pointer GC_forwardBase (const GC_state s, const pointer p);
PRIVATE void GC_markCleanliness (const GC_state s, pointer target, pointer source,
                                 char* file, int line);

#endif /* (defined (MLTON_GC_INTERNAL_BASIS)) */

//PRIVATE struct rusage* GC_getRusageGCAddr (GC_state s);

PRIVATE sigset_t* GC_getSignalsHandledAddr (GC_state *gs);
PRIVATE sigset_t* GC_getSignalsPendingAddr (GC_state *gs);
PRIVATE void GC_setGCSignalHandled (GC_state *gs, bool b);
PRIVATE bool GC_getGCSignalPending (GC_state *gs);
PRIVATE void GC_setGCSignalPending (GC_state *gs, bool b);
PRIVATE sigset_t* GC_getSignalsSet (GC_state *gs);
PRIVATE void GC_commEvent (void);
