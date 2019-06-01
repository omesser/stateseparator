#include "NCollector.h"
#include "basic_def.h"

#ifdef __NCOLLECTSTATS__
#include <ctime>

unsigned int starts[NCT_END];
unsigned int clocks[NCT_END];

void initCollector() {
	for (int i = 0; i < NCT_END; ++i) {
		starts[i] = 0;
		clocks[i] = 0;
	}
}

void startCollection(NCollectionTypes coltype) {
	starts[coltype] = clock();
}

void endCollection(NCollectionTypes coltype) {
	clocks[coltype] += (clock() - starts[coltype]);
}

double collectedTime(NCollectionTypes coltype) {
	return double(clocks[coltype])/CLOCKS_PER_SEC;
}

#endif // __NCOLLECTSTATS__
