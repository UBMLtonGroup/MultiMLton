/* Copyright (C) 1999-2008 Henry Cejtin, Matthew Fluet, Suresh
 *    Jagannathan, and Stephen Weeks.
 * Copyright (C) 1997-2000 NEC Research Institute.
 *
 * MLton is released under a BSD-style license.
 * See the file MLton-LICENSE for details.
 */

/* ---------------------------------------------------------------- */
/*                          Initialization                          */
/* ---------------------------------------------------------------- */

bool skipStackToThreadTracing = FALSE;

static bool stringToBool (char *s) {
  if (0 == strcmp (s, "false"))
    return FALSE;
  if (0 == strcmp (s, "true"))
    return TRUE;
  die ("Invalid @MLton bool: %s.", s);
}

// From gdtoa/gdtoa.h.
// Can't include the whole thing because it brings in too much junk.
float gdtoa__strtof (const char *, char **);

static float stringToFloat (char *s) {
  char *endptr;
  float f;

  f = gdtoa__strtof (s, &endptr);
  if (s == endptr)
    die ("Invalid @MLton float: %s.", s);
  return f;
}

static int32_t stringToInt (char *s) {
  char *endptr;
  int i;

  i = strtol (s, &endptr, 10);
  if (s == endptr)
    die ("Invalid @MLton int: %s.", s);
  return i;
}

static size_t stringToBytes (char *s) {
  double d;
  char *endptr;
  size_t factor;

  d = strtod (s, &endptr);
  if (s == endptr)
    goto bad;
  switch (*endptr++) {
  case 'g':
  case 'G':
    factor = 1024 * 1024 * 1024;
    break;
  case 'k':
  case 'K':
    factor = 1024;
    break;
  case 'm':
  case 'M':
    factor = 1024 * 1024;
    break;
  default:
    goto bad;
  }
  d *= factor;
  unless (*endptr == '\0'
          and 0.0 <= d
          and d <= (double)SIZE_MAX)
    goto bad;
  return (size_t)d;
bad:
  die ("Invalid @MLton memory amount: %s.", s);
}

/* ---------------------------------------------------------------- */
/*                             GC_init                              */
/* ---------------------------------------------------------------- */

int processAtMLton (GC_state s, int argc, char **argv,
                    char **worldFile) {
  int i;

  i = 1;
  while (s->controls->mayProcessAtMLton
         and i < argc
         and (0 == strcmp (argv [i], "@MLton"))) {
    bool done;

    i++;
    done = FALSE;
    while (!done) {
      if (i == argc)
        die ("Missing -- at end of @MLton args.");
      else {
        char *arg;

        arg = argv[i];
        if (0 == strcmp (arg, "copy-generational-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton copy-generational-ratio missing argument.");
          s->controls->ratios.copyGenerational = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.copyGenerational)
            die ("@MLton copy-generational-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "copy-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton copy-ratio missing argument.");
          s->controls->ratios.copy = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.copy)
            die ("@MLton copy-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "fixed-heap")) {
          i++;
          if (i == argc)
            die ("@MLton fixed-heap missing argument.");
          s->controls->fixedHeap = align (stringToBytes (argv[i++]),
                                         2 * s->sysvals.pageSize);
        } else if (0 == strcmp (arg, "gc-messages")) {
          i++;
          s->controls->messages = TRUE;
        } else if (0 == strcmp (arg, "ignore-id-for-cleanliness")) {
          i++;
          s->controls->useIdentityForCleanliness = FALSE;
        } else if (0 == strcmp (arg, "gc-summary")) {
          if (i == argc)
              die ("@MLton gc-summary missing argument");
          i++;
          if (0 == strcmp (argv[i], "cumulative"))
              s->controls->summary = SUMMARY_CUMULATIVE;
          else if (0 == strcmp (argv[i], "individual"))
              s->controls->summary = SUMMARY_INDIVIDUAL;
          else
              die ("@MLton gc-summary invalid argument");
          i++;
        } else if (0 == strcmp (arg, "alloc-chunk")) {
          i++;
          if (i == argc)
            die ("@MLton alloc-chunk missing argument.");
          s->controls->allocChunkSize = stringToBytes (argv[i++]);
          unless (GC_HEAP_LIMIT_SLOP < s->controls->allocChunkSize)
            die ("@MLton alloc-chunk argument must be greater than slop.");
        } else if (0 == strcmp (arg, "affinity-base")) {
          i++;
          if (i == argc)
            die ("@MLton affinity-base missing argument.");
          s->controls->affinityBase = stringToInt (argv[i++]);
        } else if (0 == strcmp (arg, "affinity-stride")) {
          i++;
          if (i == argc)
            die ("@MLton affinity-stride missing argument.");
          s->controls->affinityStride = stringToInt (argv[i++]);
        } else if (0 == strcmp (arg, "restrict-available")) {
          i++;
          s->controls->restrictAvailableSize = TRUE;
          fprintf (stderr, "restrict-available has been disabled\n");
          exit (1);
        } else if (0 == strcmp (arg, "available-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton available-ratio missing argument.");
          s->controls->ratios.available = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.available)
            die ("@MLton available-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "grow-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton grow-ratio missing argument.");
          s->controls->ratios.grow = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.grow)
            die ("@MLton grow-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "hash-cons")) {
          i++;
          if (i == argc)
            die ("@MLton hash-cons missing argument.");
          s->controls->ratios.hashCons = stringToFloat (argv[i++]);
          unless (0.0 <= s->controls->ratios.hashCons
                  and s->controls->ratios.hashCons <= 1.0)
            die ("@MLton hash-cons argument must be between 0.0 and 1.0.");
        } else if (0 == strcmp (arg, "live-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton live-ratio missing argument.");
          s->controls->ratios.live = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.live)
            die ("@MLton live-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "load-world")) {
          unless (s->controls->mayLoadWorld)
            die ("May not load world.");
          i++;
          s->amOriginal = FALSE;
          if (i == argc)
            die ("@MLton load-world missing argument.");
          *worldFile = argv[i++];
        } else if (0 == strcmp (arg, "mark-compact-generational-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton mark-compact-generational-ratio missing argument.");
          s->controls->ratios.markCompactGenerational = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.markCompactGenerational)
            die ("@MLton mark-compact-generational-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "mark-compact-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton mark-compact-ratio missing argument.");
          s->controls->ratios.markCompact = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.markCompact)
            die ("@MLton mark-compact-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "max-heap-local")) {
          i++;
          if (i == argc)
            die ("@MLton max-heap missing argument.");
          s->controls->maxHeapLocal = align (stringToBytes (argv[i++]),
                                       2 * s->sysvals.pageSize);
        } else if (0 == strcmp (arg, "max-heap-shared")) {
          i++;
          if (i == argc)
            die ("@MLton max-heap missing argument.");
          s->controls->maxHeapShared = align (stringToBytes (argv[i++]),
                                       2 * s->sysvals.pageSize);
        } else if (0 == strcmp (arg, "may-page-heap")) {
          i++;
          if (i == argc)
            die ("@MLton may-page-heap missing argument.");
          s->controls->mayPageHeap = stringToBool (argv[i++]);
        } else if (0 == strcmp (arg, "reclaim-objects")) {
          i++;
          if (i == argc)
            die ("@MLton may-page-heap missing argument.");
          s->controls->reclaimObjects = stringToBool (argv[i++]);
        } else if (0 == strcmp (arg, "no-load-world")) {
          i++;
          s->controls->mayLoadWorld = FALSE;
        } else if (0 == strcmp (arg, "nursery-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton nursery-ratio missing argument.");
          s->controls->ratios.nursery = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.nursery)
            die ("@MLton nursery-ratio argument must be greater than 1.0.");
        } else if (0 == strcmp (arg, "ram-slop")) {
          i++;
          if (i == argc)
            die ("@MLton ram-slop missing argument.");
          s->controls->ratios.ramSlop = stringToFloat (argv[i++]);
        } else if (0 == strcmp (arg, "show-sources")) {
          showSources (s);
          exit (0);
        } else if (0 == strcmp (arg, "stop")) {
          i++;
          s->controls->mayProcessAtMLton = FALSE;
        } else if (0 == strcmp (arg, "stack-current-grow-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton stack-current-grow-ratio missing argument.");
          s->controls->ratios.stackCurrentGrow = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.stackCurrentGrow)
            die ("@MLton stack-current-grow-ratio argument must greater than 1.0.");
        } else if (0 == strcmp (arg, "stack-current-max-reserved-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton stack-current-max-reserved-ratio missing argument.");
          s->controls->ratios.stackCurrentMaxReserved = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.stackCurrentMaxReserved)
            die ("@MLton stack-current-max-reserved-ratio argument must greater than 1.0.");
        } else if (0 == strcmp (arg, "stack-current-permit-reserved-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton stack-current-permit-reserved-ratio missing argument.");
          s->controls->ratios.stackCurrentPermitReserved = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.stackCurrentPermitReserved)
            die ("@MLton stack-current-permit-reserved-ratio argument must greater than 1.0.");
        } else if (0 == strcmp (arg, "stack-current-shrink-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton stack-current-shrink-ratio missing argument.");
          s->controls->ratios.stackCurrentShrink = stringToFloat (argv[i++]);
          unless (0.0 <= s->controls->ratios.stackCurrentShrink
                  and s->controls->ratios.stackCurrentShrink <= 1.0)
            die ("@MLton stack-current-shrink-ratio argument must be between 0.0 and 1.0.");
        } else if (0 == strcmp (arg, "stack-max-reserved-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton stack-max-reserved-ratio missing argument.");
          s->controls->ratios.stackMaxReserved = stringToFloat (argv[i++]);
          unless (1.0 < s->controls->ratios.stackMaxReserved)
            die ("@MLton stack-max-reserved-ratio argument must greater than 1.0.");
        } else if (0 == strcmp (arg, "stack-shrink-ratio")) {
          i++;
          if (i == argc)
            die ("@MLton stack-shrink-ratio missing argument.");
          s->controls->ratios.stackShrink = stringToFloat (argv[i++]);
          unless (0.0 <= s->controls->ratios.stackShrink
                  and s->controls->ratios.stackShrink <= 1.0)
            die ("@MLton stack-shrink-ratio argument must be between 0.0 and 1.0.");
        } else if (0 == strcmp (arg, "use-mmap")) {
          i++;
          GC_setCygwinUseMmap (TRUE);
        } else if (0 == strcmp (arg, "number-processors")) {
          i++;
          if (i == argc)
            die ("@MLton number-processors missing argument.");
          if (!s->amOriginal)
            die ("@MLton number-processors incompatible with loaded worlds.");
          s->numberOfProcs = stringToFloat (argv[i++]) + s->numIOThreads;
          /* Turn off loaded worlds -- they are unsuppoed in multi-proc mode */
          s->controls->mayLoadWorld = FALSE;
        } else if (0 == strcmp (arg, "io-threads")) {
          i++;
          if (i == argc)
            die ("@MLton io-threads missing argument.");
          s->numIOThreads = stringToFloat (argv[i++]);
          s->numberOfProcs += s->numIOThreads;
        } else if (0 == strcmp (arg, "enable-timer")) {
          i++;
          if (i == argc)
            die ("@MLton enable-timer missing argument.");
          s->enableTimer = TRUE;
          s->timeInterval = stringToFloat (argv[i++]);
        } else if (0 == strcmp (arg, "--")) {
          i++;
          done = TRUE;
        } else if (i > 1)
          die ("Strange @MLton arg: %s", argv[i]);
        else done = TRUE;
      }
    }
  }
  return i;
}

static inline void* initCumulativeStatistics (void) {
  struct GC_cumulativeStatistics* cumul =
      (struct GC_cumulativeStatistics *) malloc (sizeof (struct GC_cumulativeStatistics));
  cumul->bytesAllocated = 0;
  cumul->bytesFilled = 0;
  cumul->bytesCopied = 0;
  cumul->bytesCopiedShared = 0;
  cumul->bytesCopiedMinor = 0;
  cumul->bytesHashConsed = 0;
  cumul->bytesLifted = 0;
  cumul->bytesMarkCompacted = 0;
  cumul->bytesMarkCompactedShared = 0;
  cumul->bytesScannedMinor = 0;
  cumul->maxBytesLive = 0;
  cumul->maxSharedBytesLive = 0;
  cumul->maxBytesLiveSinceReset = 0;
  cumul->maxHeapSize = 0;
  cumul->maxSharedHeapSize = 0;
  cumul->maxPauseTime = 0;
  cumul->maxStackSize = 0;
  cumul->numCardsMarked = 0;
  cumul->numLimitChecks = 0;
  cumul->syncForOldGenArray = 0;
  cumul->syncForNewGenArray = 0;
  cumul->syncForStack = 0;
  cumul->syncForHeap = 0;
  cumul->syncForLift = 0;
  cumul->syncForce = 0;
  cumul->syncMisc = 0;
  cumul->syncForLiftNoGC = 0;
  cumul->numCopyingGCs = 0;
  cumul->numCopyingSharedGCs = 0;
  cumul->numSharedGCs = 0;
  cumul->numHashConsGCs = 0;
  cumul->numMarkCompactGCs = 0;
  cumul->numMarkCompactSharedGCs = 0;
  cumul->numMinorGCs = 0;
  cumul->numThreadsCreated = 0;
  cumul->bytesThreadReserved = 0;
  cumul->countThreadReserved = 0;
  cumul->bytesThreadUsed = 0;
  cumul->countThreadUsed = 0;
  cumul->numForceStackGrowth = 0;
  cumul->numPreemptWB = 0;
  cumul->numMoveWB = 0;
  cumul->numReadyPrimWB = 0;
  cumul->numReadySecWB = 0;
  cumul->numPreemptGC  = 0;
  cumul->numReadyPrimGC = 0;
  cumul->numReadySecGC = 0;

  cumul->bytesParasiteStack = 0;
  cumul->bytesParasiteClosure = 0;
  cumul->numParasitesReified = 0;
  cumul->numParasitesCreated = 0;
  cumul->numRBChecks = 0;
  cumul->numRBChecksForwarded = 0;
  cumul->cyclesRB = 0;

  cumul->numComms = 0;

  timevalZero (&cumul->ru_gc);
  rusageZero (&cumul->ru_gcCopying);
  rusageZero (&cumul->ru_gcCopyingShared);
  rusageZero (&cumul->ru_gcMarkCompact);
  rusageZero (&cumul->ru_gcMarkCompactShared);
  rusageZero (&cumul->ru_gcMinor);
  timevalZero (&cumul->tv_sync);
  rusageZero (&cumul->ru_thread);
  timevalZero (&cumul->tv_serial);
  timevalZero (&cumul->tv_rt);
  return (void*)cumul;
}

static inline void* initLastMajorStatistics (void) {
  struct GC_lastMajorStatistics* cumul = (struct GC_lastMajorStatistics*)
    malloc (sizeof (struct GC_lastMajorStatistics));
  cumul->bytesHashConsed = 0;
  cumul->bytesLive = 0;
  cumul->kind = GC_COPYING;
  cumul->numMinorGCs = 0;
  return (void*)cumul;
}

static inline void* initLastSharedMajorStatistics (void) {
  struct GC_lastSharedMajorStatistics* cumul =
    (struct GC_lastSharedMajorStatistics*) malloc (sizeof (struct GC_lastSharedMajorStatistics));
  cumul->bytesLive = 0;
  cumul->kind = GC_COPYING;
  return (void*)cumul;
}


int GC_init (GC_state s, int argc, char **argv) {
  int res;

  assert (s->alignment >= GC_MODEL_MINALIGN);
  assert (isAligned (sizeof (struct GC_stack), s->alignment));

  s->amInGC = TRUE;
  s->amOriginal = TRUE;
  s->atomicState = 0;
  s->callFromCHandlerThread = BOGUS_OBJPTR;
  s->controls = (struct GC_controls *) malloc (sizeof (struct GC_controls));
  s->controls->fixedHeap = 0;
  s->controls->maxHeapLocal = 0;
  s->controls->maxHeapShared = 0;
  s->controls->mayLoadWorld = TRUE;
  s->controls->mayPageHeap = FALSE;
  s->controls->mayProcessAtMLton = TRUE;
  s->controls->messages = FALSE;
  s->controls->oldGenArraySize = 0x100000;
  s->controls->allocChunkSize = 4096;
  s->controls->affinityBase = 0;
  s->controls->affinityStride = 1;
  s->controls->restrictAvailableSize = FALSE;
  s->controls->ratios.copy = 4.0;
  s->controls->ratios.copyGenerational = 4.0;
  s->controls->ratios.grow = 8.0;
  s->controls->ratios.hashCons = 0.0;
  s->controls->ratios.live = 8.0;
  s->controls->ratios.markCompact = 1.04;
  s->controls->ratios.markCompactGenerational = 8.0;
  s->controls->ratios.nursery = 10.0;
  s->controls->ratios.ramSlop = 0.5;
  s->controls->ratios.available = 1.1;
  s->controls->ratios.stackCurrentGrow = 2.0;
  s->controls->ratios.stackCurrentMaxReserved = 32.0;
  s->controls->ratios.stackCurrentPermitReserved = 4.0;
  s->controls->ratios.stackCurrentShrink = 0.5;
  s->controls->ratios.stackMaxReserved = 8.0;
  s->controls->ratios.stackShrink = 0.5;
  s->selectiveDebug = FALSE;
  s->controls->summary = SUMMARY_NONE;
  s->controls->reclaimObjects = FALSE;
  s->controls->useIdentityForCleanliness = TRUE;

  s->forwardState.liftingObject = BOGUS_OBJPTR;

  // While the following asserts are manifestly true,
  // they check the asserts in sizeofThread and sizeofWeak.
  assert (sizeofThread (s) == sizeofThread (s));
  assert (sizeofWeak (s) == sizeofWeak (s));

  s->cumulativeStatistics = (struct GC_cumulativeStatistics*)initCumulativeStatistics ();
  s->lastMajorStatistics = (struct GC_lastMajorStatistics*)initLastMajorStatistics ();
  s->lastSharedMajorStatistics = (struct GC_lastSharedMajorStatistics*)initLastSharedMajorStatistics ();
  s->currentThread = BOGUS_OBJPTR;
  s->danglingStackList = NULL;
  s->hashConsDuringGC = FALSE;
  s->heap = (GC_heap) malloc (sizeof (struct GC_heap));
  initHeap (s, s->heap, LOCAL_HEAP);
  s->numberOfProcs = 1;
  s->numIOThreads = 0;
  s->enableTimer = FALSE;
  s->timeInterval = 200;
  s->copiedSize = -1;
  s->procStates = NULL;
  s->roots = NULL;
  s->rootsLength = 0;
  s->savedThread = BOGUS_OBJPTR;
  s->savedClosure = BOGUS_OBJPTR;
  s->pacmlThreadId = BOGUS_OBJPTR;
  s->secondaryLocalHeap = (GC_heap) malloc (sizeof (struct GC_heap));
  initHeap (s, s->secondaryLocalHeap, LOCAL_HEAP);
  s->sharedHeap = (GC_heap) malloc (sizeof (struct GC_heap));
  initHeap (s, s->sharedHeap, SHARED_HEAP);
  s->sharedHeapStart = 0;
  s->sharedHeapEnd = 0;
  s->secondarySharedHeap = (GC_heap) malloc (sizeof (struct GC_heap));
  initHeap (s, s->secondarySharedHeap, SHARED_HEAP);
  s->signalHandlerThread = BOGUS_OBJPTR;
  s->signalsInfo.amInSignalHandler = FALSE;
  s->signalsInfo.gcSignalHandled = FALSE;
  s->signalsInfo.gcSignalPending = FALSE;
  s->signalsInfo.signalIsPending = FALSE;
  sigemptyset (&s->signalsInfo.signalsHandled);
  sigemptyset (&s->signalsInfo.signalsPending);
  s->startTime = getCurrentTime ();
  s->syncReason = SYNC_NONE;
  s->sysvals.pageSize = GC_pageSize ();
  s->sysvals.physMem = GC_physMem ();
  s->weaks = NULL;
  s->saveWorldStatus = true;
  s->profiling.isProfilingTimeOn = FALSE;
  s->reachable = NULL;
  s->copyImmutable = FALSE;
  s->copyObjectMap = NULL;
  utarray_new (s->directCloXferArray, &directCloXfer_icd);

  s->schedulerQueue = NULL;
  s->schedulerLocks = NULL;

  s->danglingStackList = (objptr*) malloc (sizeof (objptr) * BUFFER_SIZE);
  s->danglingStackListSize = 0;
  s->danglingStackListMaxSize = BUFFER_SIZE;

  s->moveOnWBA = (objptr*) malloc (sizeof (objptr) * BUFFER_SIZE);
  s->moveOnWBASize = 0;
  s->moveOnWBAMaxSize = BUFFER_SIZE;

  s->preemptOnWBA = (PreemptThread*) malloc (sizeof (PreemptThread) * BUFFER_SIZE);
  s->preemptOnWBASize = 0;
  s->preemptOnWBAMaxSize = BUFFER_SIZE;

  s->spawnOnWBA = (SpawnThread*) malloc (sizeof (SpawnThread) * BUFFER_SIZE);
  s->spawnOnWBASize = 0;
  s->spawnOnWBAMaxSize = BUFFER_SIZE;

  s->tmpBool = FALSE;
  s->tmpPointer = BOGUS_POINTER;
  s->tmpInt = -1;

  s->translateState.from = BOGUS_POINTER;
  s->translateState.to = BOGUS_POINTER;
  s->translateState.size = 0;


  initIntInf (s);
  initSignalStack (s);
  s->worldFile = NULL;

  unless (isAligned (s->sysvals.pageSize, CARD_SIZE))
    die ("Page size must be a multiple of card size.");
  processAtMLton (s, s->atMLtonsLength, s->atMLtons, &s->worldFile);
  res = processAtMLton (s, argc, argv, &s->worldFile);
  if (s->controls->fixedHeap > 0 and s->controls->maxHeapLocal > 0)
    die ("Cannot use both fixed-heap and max-heap.");
  unless (s->controls->ratios.markCompact <= s->controls->ratios.copy
          and s->controls->ratios.copy <= s->controls->ratios.live)
    die ("Ratios must satisfy mark-compact-ratio <= copy-ratio <= live-ratio.");
  unless (s->controls->ratios.stackCurrentPermitReserved
          <= s->controls->ratios.stackCurrentMaxReserved)
    die ("Ratios must satisfy stack-current-permit-reserved <= stack-current-max-reserved.");

  Parallel_initResources (s);
  GC_sqCreateQueues (s);
  /* We align s->ram by pageSize so that we can test whether or not we
   * we are using mark-compact by comparing heap size to ram size.  If
   * we didn't round, the size might be slightly off.
   */
  uintmax_t ram;
  ram = alignMax ((uintmax_t)(s->controls->ratios.ramSlop * s->sysvals.physMem),
                  (uintmax_t)(s->sysvals.pageSize));
  ram = min (ram, (uintmax_t)SIZE_MAX);
  s->sysvals.ram = (size_t)ram;
  if (DEBUG or DEBUG_RESIZING or s->controls->messages)
    fprintf (stderr, "[GC: Found %s bytes of RAM; using %s bytes (%.1f%% of RAM).]\n",
             uintmaxToCommaString(s->sysvals.physMem),
             uintmaxToCommaString(s->sysvals.ram),
             100.0 * ((double)ram / (double)(s->sysvals.physMem)));
  if (DEBUG_SOURCES or DEBUG_PROFILE) {
    uint32_t i;
    for (i = 0; i < s->sourceMaps.frameSourcesLength; i++) {
      uint32_t j;
      uint32_t *sourceSeq;
      fprintf (stderr, "%"PRIu32"\n", i);
      sourceSeq = s->sourceMaps.sourceSeqs[s->sourceMaps.frameSources[i]];
      for (j = 1; j <= sourceSeq[0]; j++)
        fprintf (stderr, "\t%s\n",
                 s->sourceMaps.sourceNames[s->sourceMaps.sources[sourceSeq[j]].sourceNameIndex]);
    }
  }
  return res;
}

void GC_lateInit (GC_state s) {
  /* Initialize profiling.  This must occur after processing
   * command-line arguments, because those may just be doing a
   * show-sources, in which case we don't want to initialize the
   * atExit.
   */

  initProfiling (s);

  if (s->amOriginal) {
    initWorld (s);
    /* The mutator stack invariant doesn't hold,
     * because the mutator has yet to run.
     */
    // spoons: can't assert because other threads are init'd
    //assert (invariantForMutator (s, TRUE, FALSE));
  } else {
    loadWorldFromFileName (s, s->worldFile);
    if (s->profiling.isOn and s->profiling.stack)
      foreachStackFrame (s, enterFrameForProfiling);
    assert (invariantForMutator (s, TRUE, TRUE));
  }
  s->amInGC = FALSE;
}

void GC_duplicate (GC_state d, GC_state s) {
  // GC_init
  d->selectiveDebug = FALSE;
  d->amInGC = s->amInGC;
  d->amOriginal = s->amOriginal;
  d->atomicState = 0;
  d->callFromCHandlerThread = BOGUS_OBJPTR;
  d->controls = s->controls;
  d->cumulativeStatistics = initCumulativeStatistics ();
  d->forwardState.liftingObject = BOGUS_OBJPTR;
  d->lastMajorStatistics = initLastMajorStatistics ();
  d->lastSharedMajorStatistics = s->lastSharedMajorStatistics;
  d->currentThread = BOGUS_OBJPTR;
  d->hashConsDuringGC = s->hashConsDuringGC;
  d->numberOfProcs = s->numberOfProcs;
  d->numIOThreads = s->numIOThreads;
  d->enableTimer = s->enableTimer;
  d->timeInterval = s->timeInterval;
  d->sharedHeapStart = s->sharedHeapStart;
  d->sharedHeapEnd = s->sharedHeapEnd;
  d->roots = NULL;
  d->rootsLength = 0;
  d->savedThread = BOGUS_OBJPTR;
  d->savedClosure = BOGUS_OBJPTR;
  d->pacmlThreadId = BOGUS_OBJPTR;
  d->signalHandlerThread = BOGUS_OBJPTR;
  d->signalsInfo.amInSignalHandler = FALSE;
  d->signalsInfo.gcSignalHandled = FALSE;
  d->signalsInfo.gcSignalPending = FALSE;
  d->signalsInfo.signalIsPending = FALSE;
  sigemptyset (&d->signalsInfo.signalsHandled);
  sigemptyset (&d->signalsInfo.signalsPending);
  d->startTime = s->startTime;
  d->syncReason = SYNC_NONE;
  d->sysvals.physMem = s->sysvals.physMem;
  d->sysvals.pageSize = s->sysvals.pageSize;
  d->weaks = s->weaks;
  d->copiedSize = s->copiedSize;
  d->saveWorldStatus = s->saveWorldStatus;
  d->reachable = NULL;
  d->forwardState.liftingObject = BOGUS_OBJPTR;
  d->schedulerLocks = NULL;
  d->schedulerQueue = NULL;
  d->moveOnWBA = (objptr*) malloc (sizeof (objptr) * BUFFER_SIZE);
  d->moveOnWBASize = 0;
  d->moveOnWBAMaxSize = BUFFER_SIZE;
  d->danglingStackList = (objptr*) malloc (sizeof (objptr) * BUFFER_SIZE);
  d->danglingStackListSize = 0;
  d->danglingStackListMaxSize = BUFFER_SIZE;
  d->preemptOnWBA = (PreemptThread*) malloc (sizeof (PreemptThread) * BUFFER_SIZE);
  d->preemptOnWBASize = 0;
  d->preemptOnWBAMaxSize = BUFFER_SIZE;
  d->spawnOnWBA = (SpawnThread*) malloc (sizeof (SpawnThread) * BUFFER_SIZE);
  d->spawnOnWBASize = 0;
  d->spawnOnWBAMaxSize = BUFFER_SIZE;
  d->translateState.from = BOGUS_POINTER;
  d->translateState.to = BOGUS_POINTER;
  d->translateState.size = 0;
  utarray_new (d->directCloXferArray, &directCloXfer_icd);
  d->sysvals.ram = s->sysvals.ram;
  d->copyObjectMap = NULL;

  assert (d->amOriginal);
  duplicateWorld (d, s);
  s->amInGC = FALSE;
}
