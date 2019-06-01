/*
 * The following code is for native testing during development.
 * Matrix randomization is done on the server using the NRandomizer executable.
 * See Randomizer project in this solution and the makefile for Linux.
 */
#ifndef __NRUNONSERVER__

#ifndef N_TESTER_H
#define N_TESTER_H

enum ExampleMatrices {
	NEM_BARELY_SEPARABLE,
	NEM_SLIGHTLY_ENTANGLED,
	NEM_BELL_1,
	NEM_BELL_2,
	NEM_SIMPLE_SEP,  // A simple separable matrix to be separatable to 2 states
	NEM_END	// for enumerations purposes only
}; // ExampleMatrices


/*****************************************************************************
 *                       GLOBAL FUNCTIONS DECLARATIONS                       *
 *****************************************************************************/
void runExample(ExampleMatrices example);
void testRandom(const char* particleSizesStr = "2 2");

#endif // N_TESTER_H

#endif // __NRUNONSERVER__
