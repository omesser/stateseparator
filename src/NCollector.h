#ifndef N_COLLECTOR_H
#define N_COLLECTOR_H

enum NCollectionTypes {
	NCT_TOTAL_TIME = 0,
	NCT_MINIMIZATION,
	NCT_ADD_STATE,
	NCT_SHIFT,
	NCT_LINEAR_SOLVE,
	NCT_BLOCK_ALLOCATION,
	NCT_UPDATE_PROBS,
	NCT_BUILD_MATRIX,
	NCT_MIXIN,
	NCT_MAIN_ITERATION,
	NCT_FNPS,
	NCT_CALC_EV,
	NCT_PTO,
	NCT_END
};

/*****************************************************************************
 *                             COLLECTION MACROS                             *
 *****************************************************************************/

#ifdef __NCOLLECTSTATS__
void initCollector();
void startCollection(NCollectionTypes coltype);
void endCollection(NCollectionTypes coltype);
double collectedTime(NCollectionTypes coltype);

#define NINITCOLLECTOR initCollector();
#define NSTARTCOLLECTION(ct) startCollection(ct);
#define NENDCOLLECTION(ct) endCollection(ct);

#else  // __NCOLLECTSTATS__ not defined
#define NINITCOLLECTOR
#define NSTARTCOLLECTION(ct)
#define NENDCOLLECTION(ct)

#endif  // __NCOLLECTSTATS__ not defined

#endif  // N_COLLECTOR_H
