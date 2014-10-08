/*
 * gc_state_global.h
 *
 *  Created on: Oct 6, 2014
 *      Author: korpy
 */

#ifndef RUNTIME_GC_GC_STATE_GLOBAL_H_
#define RUNTIME_GC_GC_STATE_GLOBAL_H_

#if (defined (MLTON_GC_INTERNAL_TYPES))
struct GC_state_global
{
	pointer sessionStart;
	pointer sharedHeapStart;
	pointer sharedHeapEnd;
	pointer sharedFrontier;
	pointer sharedLimit;

	char **atMLtons; /* Initial @MLton args, processed before command line. *///global
	uint32_t atMLtonsLength; //global
	objptr callFromCHandlerThread; /* Handler for exported C calls (in heap). *///global?//global
	struct GC_controls *controls; //global
	struct GC_cumulativeStatistics *cumulativeStatistics; //global
	int (*loadGlobals)(FILE *f); /* loads the globals from the file. *///global
	uint32_t magic; /* The magic number for this executable. *///global
	uint32_t maxFrameSize;//global
	bool selectiveDebug;//global

	GC_state procStates;
	struct GC_heap *sharedHeap; /* Used as a uncollected shared heap for testing lwtgc */
	struct GC_heap *secondarySharedHeap; /* Used for major copying collection on shared heap */
};
#endif /* (defined (MLTON_GC_INTERNAL_TYPES)) */
#endif /* RUNTIME_GC_GC_STATE_GLOBAL_H_ */
