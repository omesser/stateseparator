#ifndef N_PURE_STATE_H
#define N_PURE_STATE_H

#include "basic_def.h"
#include "NMathUtils.h"

class NPureState;
class NMixedState;


/*****************************************************************************
 *                              GLOBAL FUNCTIONS                             *
 *****************************************************************************/
inline MatrixXcd operator*(const NPureState& state, const double& scalar);
inline MatrixXcd operator*(const double& scalar, const NPureState& state);
inline MatrixXcd operator*(const NPureState& lhs, const NPureState& rhs);
inline MatrixXcd operator*(const NPureState& lhs, const MatrixXcd& rhs);
inline MatrixXcd operator*(const MatrixXcd& lhs, const NPureState& rhs);
inline ValType trace(const NPureState& lhs, const NPureState& rhs);
inline ValType trace(const NPureState& lhs, const MatrixXcd& rhs);
inline ValType trace(const MatrixXcd& lhs, const NPureState& rhs);


/*****************************************************************************
 *                              Class NPureState                             *
 *****************************************************************************/
class NPureState {
public:
	// Constructors
	inline NPureState();
	NPureState(const vector<uint>& particleSizes);
	inline NPureState(const NPureState& src);

	// Assignment operators
	NPureState& operator=(const NPureState& src);
	NPureState& operator=(const vector<uint>& particleSizes);
	inline void finalize();

	// Comparison operators
	bool operator<(const NPureState& src) const;

	// Queries
	inline uint size() const; // overall size of the state
	inline uint numOfParticles() const; // number of basic particles that the state is built of
	inline const MatrixXcd& matrix() const;

	// Direct access operators:
	inline VectorXcd& operator[](uint i);
	inline const VectorXcd& operator[](uint i) const;

	// Output functions
	void print(const double& prob, uint num) const;

private:
	uint _size; // overall size of the state
	vector<VectorXcd> _particles; // the individual particles that the state is built of
	VectorXcd _vectorForm;
	MatrixXcd _matrixForm;
	bool _matrixBuilt;

	friend MatrixXcd operator*(const NPureState& state, const double& scalar);
	friend MatrixXcd operator*(const double& scalar, const NPureState& state);
	friend MatrixXcd operator*(const NPureState& lhs, const NPureState& rhs);
	friend MatrixXcd operator-(const NMixedState& lhs, const NPureState& rhs);
	friend MatrixXcd operator-(const NPureState& lhs, const NMixedState& rhs);
	friend MatrixXcd operator*(const NPureState& lhs, const MatrixXcd& rhs);
	friend MatrixXcd operator*(const MatrixXcd& lhs, const NPureState& rhs);
	friend ValType trace(const NPureState& lhs, const NPureState& rhs);
	friend ValType trace(const NPureState& lhs, const MatrixXcd& rhs);
	friend ValType trace(const MatrixXcd& lhs, const NPureState& rhs);

	void buildMatrix();
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NPureState::NPureState() : _size(0), _matrixBuilt(false) {}

NPureState::NPureState(const NPureState& src)
: _size(src._size), _particles(src._particles), _vectorForm(src._vectorForm),
  _matrixForm(src._matrixForm), _matrixBuilt(src._matrixBuilt)
{
}

void NPureState::finalize() {
	buildMatrix();
}

uint NPureState::size() const {
	return _size;
}

uint NPureState::numOfParticles() const {
	return (uint)_particles.size();
}

const MatrixXcd& NPureState::matrix() const {
	if (!_matrixBuilt) {
		throw ("NPureState: State is not finalized - no matrix.");
	}
	return _matrixForm;
}

VectorXcd& NPureState::operator[](uint i) {
	_matrixBuilt = false;
	return _particles[i];
}

const VectorXcd& NPureState::operator[](uint i) const {
	return _particles[i];
}


/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
MatrixXcd operator*(const NPureState& state, const double& scalar) {
	return (state.matrix() * scalar);
}

MatrixXcd operator*(const double& scalar, const NPureState& state) {
	return state.matrix() * scalar;
}

MatrixXcd operator*(const NPureState& lhs, const NPureState& rhs) {
	return lhs.matrix().selfadjointView<Eigen::Lower>() * rhs.matrix();
}

MatrixXcd operator*(const NPureState& lhs, const MatrixXcd& rhs) {
	return lhs.matrix().selfadjointView<Eigen::Lower>() * rhs;
}

MatrixXcd operator*(const MatrixXcd& lhs, const NPureState& rhs) {
	return lhs * rhs.matrix().selfadjointView<Eigen::Lower>();
}

ValType trace(const NPureState& lhs, const NPureState& rhs) {
	return trace(lhs.matrix(), rhs.matrix());
}

ValType trace(const NPureState& lhs, const MatrixXcd& rhs) {
	return trace(lhs.matrix(), rhs);
}

ValType trace(const MatrixXcd& lhs, const NPureState& rhs) {
	return trace(lhs, rhs.matrix());
}

#endif // N_PURE_STATE_H
