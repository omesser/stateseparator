#ifndef N_RANDOM_GENERATOR_H
#define N_RANDOM_GENERATOR_H

#include <ctime>
#include "basic_def.h"


/*****************************************************************************
 *                           Class NRandomGenerator                          *
 *****************************************************************************/
class NRandomGenerator {
public:
	inline static NRandomGenerator* getInstance();

#ifdef __NNORANDOM__
	inline static void create(uint seed);
#endif  // __NNORANDOM__

	inline uint seed() const;

	void randomizeVector(VectorXcd& vec, bool pureReal = false) const;
	void randomizeProbabilities(VectorXd& vec) const;

	// Generate a random number between 1 and maxNum. We assume that maxNum > 1
	inline uint getUint(uint maxNum = RAND_MAX) const;

private:
	// The singleton instance
	static NRandomGenerator* _theInstance;

	// Data members
	uint _seed;

	// Constrtuctor
	NRandomGenerator(uint seed = time(0));

	// Utility functions
	inline double val() const;  // returns a random number in [-1, 1]

	// Masking copy constructor and assignment operator
	inline NRandomGenerator(const NRandomGenerator& src);  // ERROR
	inline void operator=(const NRandomGenerator& src);    // ERROR
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NRandomGenerator* NRandomGenerator::getInstance() {
	if (_theInstance == NULL) {
		try {
			_theInstance = new NRandomGenerator();
		} catch (bad_alloc) {
			throw NError("NRandomGenerator: Unable to create the NRandomGenerator instance.");
		}
	}
	return _theInstance;
}

#ifdef __NNORANDOM__
void NRandomGenerator::create(uint seed) {
	if (_theInstance == NULL) {
		try {
			_theInstance = new NRandomGenerator(seed);
		} catch (bad_alloc) {
			throw NError("NRandomGenerator: Unable to create the NRandomGenerator instance.");
		}
	}
}
#endif  // __NNORANDOM__

uint NRandomGenerator::seed() const {
	return _seed;
}

uint NRandomGenerator::getUint(uint maxNum /*= RAND_MAX*/) const {
	return (uint(((double(rand()) / RAND_MAX) * (maxNum - 1))) + 1);
}

/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
double NRandomGenerator::val() const {
	return (((double(rand()) / RAND_MAX) * 2) - 1);
}

NRandomGenerator::NRandomGenerator(const NRandomGenerator& src) {
	throw NError("NRandomGenerator: Copy constructor was invoked.");
}

void NRandomGenerator::operator=(const NRandomGenerator& src) {
	throw NError("NRandomGenerator: Assignment operator was invoked.");
}

#endif  // N_RANDOM_GENERATOR_H
