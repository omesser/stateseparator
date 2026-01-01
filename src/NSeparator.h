#ifndef N_SEPARATOR_H
#define N_SEPARATOR_H

#include <Eigen/Eigenvalues>
#include "NMixedState.h"
#include "NRandomGenerator.h"
#include "NPeresTester.h"

typedef Eigen::SelfAdjointEigenSolver<MatrixXcd> EVSolver;

const double targetDistanceDefault = 0.5E-13;


/*****************************************************************************
 *                               Struct NResult                              *
 *****************************************************************************/
enum NReason { NREASON_NONE = 0, NREASON_DISTANCE, NREASON_S, NREASON_TIMEOUT, NREASON_FAIL };
const string ReasonString[] = {
    "unknown reason",                          // NREASON_NONE
    "target distance reached",                 // NREASON_DISTANCE
    "algorithm convergence criteria",          // NREASON_S
    "maximum number of iterations performed",  // NREASON_TIMEOUT
    "numerical error"                          // NREASON_FAIL
};

struct NResult {
	inline NResult(const NMixedState& state, const double& distance);

	VectorXcd _peresEVs;
	ValType _minPeresEV;
	NMixedState _state;
	double _distance;
	NReason _reason;
};

NResult::NResult(const NMixedState& state, const double& distance)
    : _state(state), _distance(distance), _reason(NREASON_NONE) {
	_peresEVs = VectorXcd();
	_minPeresEV = ValType();
}


/*****************************************************************************
 *                              Class NSeparator                             *
 *****************************************************************************/
class NSeparator {
public:
	inline static NSeparator* getInstance();

	NResult separate(const MatrixXcd& src, const vector<uint>& particleSizes, double targetDistance,
	                 double minProbForState, uint targetNumberOfStates, bool boostAccuracy = false);

private:
	// The singleton instance
	static NSeparator* _theInstance;

	// Type definitions
	typedef vector<uint> IndexVector;
	typedef vector<MatrixXcd> MatrixVector;

	class ParticleRecord {
	public:
		inline ParticleRecord() : _size(0), _index(~0) {}  // ~0 = max uint
		inline ParticleRecord(uint size, uint index) : _size(size), _index(index) {}
		inline void operator=(const ParticleRecord& src) {
			_size = src._size;
			_index = src._index;
		}
		uint completeRecord(uint jumpFactor, uint systemSize);
		inline bool operator==(const ParticleRecord& cmp) const {
			return (_index == cmp._index);
		}
		inline bool operator<(const ParticleRecord& cmp) const {
			if (_size == cmp._size)
				return (_index < cmp._index);
			return (_size < cmp._size);
		}
		void partialTraceout(const MatrixXcd& matrix, const VectorXcd& vec);

		uint _size;            // Particle size
		uint _index;           // The particle's actual place in the system
		uint _jumpFactor;      // The jump factor for traceouts
		IndexVector _indices;  // Indices for the partial traceouts
		MatrixXcd _subMatrix;  // Preallocated matrix to be used in the traceouts - to avoid repeated allocations

	private:
		void buildIndices(uint systemSize);
	};

	typedef vector<ParticleRecord> SystemStructure;

	// Data fields
	NRandomGenerator* _gen;
	vector<EVSolver> _solvers;
	MatrixXcd rho;
	NPureState rho_p;
	uint _numOfParticles;
	uint _systemSize;
	vector<SystemStructure> _traceoutData;
	NPeresTester _peresTester;

	// Constructor
	inline NSeparator();

	// Main algorithm functions
	void reset(const MatrixXcd& src, const vector<uint>& particleSizes, double& minProbForState,
	           uint& targetNumberOfStates, uint fnpsIterationNum);
	double mainIterationStep(const NMixedState& rho_s, uint fnpsIterationNum);

	// Find nearest pure state
	void findNearestPureState(const MatrixXcd& sigma, uint fnpsIterationNum);
	bool calcFNPSIteration(const MatrixXcd& sigma);
	void traceOut(const MatrixXcd& sigma, uint particleNum);

	// Utility functions
	double maxEigenVector(EVSolver& solver, const MatrixXcd& mat, VectorXcd& eigenVector);
	void buildSystemData(const vector<uint>& particleSizes);
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NSeparator* NSeparator::getInstance() {
	if (_theInstance == NULL) {
		try {
			_theInstance = new NSeparator();
		} catch (bad_alloc) {
			throw NError("NSeparator: Unable to create the NSeparator instance.");
		}
	}
	return _theInstance;
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
NSeparator::NSeparator() : _gen(NRandomGenerator::getInstance()) {}

#endif  // N_SEPARATOR_H
