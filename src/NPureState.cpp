#include "NPureState.h"
#include "NOutputHandler.h"


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NPureState::NPureState(const vector<uint>& particleSizes)
    : _size(1), _particles(particleSizes.size()), _matrixBuilt(false) {
	for (uint i = 0; i < _particles.size(); ++i) {
		_particles[i].setZero(particleSizes[i]);
		_size *= particleSizes[i];
	}
}

NPureState& NPureState::operator=(const NPureState& src) {
	if (this != &src) {
		_size = src._size;
		_particles = src._particles;
		_matrixBuilt = src._matrixBuilt;
		if (_matrixBuilt) {
			_vectorForm = src._vectorForm;
			_matrixForm = src._matrixForm;
		}
	}
	return *this;
}

NPureState& NPureState::operator=(const vector<uint>& particleSizes) {
	_matrixBuilt = false;
	if (particleSizes.empty()) {
		_size = 0;
		return *this;
	}
	_size = 1;
	_particles.resize(particleSizes.size());
	for (uint i = 0; i < particleSizes.size(); ++i) {
		_size *= particleSizes[i];
		_particles[i].resize(particleSizes[i]);
	}
	return *this;
}

bool NPureState::operator<(const NPureState& src) const {
#ifdef __NSTRICT__
	if (_size != src._size || _particles.size() != src._particles.size()) {
		throw NError("NPureState: Unable to compare states of different dimensions.");
	}
	for (uint i = 0; i < _particles.size(); ++i) {
		if (_particles[i].size() != src._particles[i].size()) {
			throw NError("NPureState: Unable to compare states of different dimensions.");
		}
	}
#endif  // __NSTRICT__
	for (uint i = 0; i < _particles.size(); ++i) {
		for (int j = 0; j < _particles[i].size(); ++j) {
			if (isEqual(_particles[i][j], src._particles[i][j]))
				continue;
			return (src._particles[i][j] < _particles[i][j]);  // this is intentionally inverted
		}
	}
	return false;  // states are identical
}

void NPureState::print(const double& prob, uint num) const {
	if (!_matrixBuilt) {
		throw("NPureState: State is not finalized - no vector form.");
	}
	stringstream printstrm;
	printstrm << "#" << num << ")\tweight: ";
	NOutputHandler::printStrm(printstrm, prob);
	printstrm << endl << "\tstate: ";
	NOutputHandler::printStrm(printstrm, _particles[0]);
	uint numOfParticles = (uint)_particles.size();
	for (uint i = 1; i < numOfParticles; ++i) {
		printstrm << " X ";
		NOutputHandler::printStrm(printstrm, _particles[i]);
	}
	NDATA(printstrm);
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
void NPureState::buildMatrix() {
	if (_particles.empty())
		return;
	if (_matrixBuilt)
		return;
	uint numOfParticles = (uint)_particles.size();
	_vectorForm = _particles[0];
	for (uint i = 1; i < numOfParticles; ++i) {
		_vectorForm = tensorProduct(_vectorForm, _particles[i]);
	}
	_matrixForm.noalias() = (_vectorForm * _vectorForm.adjoint());
	_matrixBuilt = true;
}
