#pragma once
#ifndef N_MATH_UTILS_H
#define N_MATH_UTILS_H

#include <cmath>
#include <complex>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>

using std::complex;
using std::vector;
using std::pow;
using std::sqrt;
using std::abs;

using Eigen::MatrixXcd;
using Eigen::MatrixXd;
using Eigen::VectorXcd;
using Eigen::VectorXd;

typedef unsigned int uint;
typedef complex<double> ValType;

// These constants are used as an approximation of zero for floating point calculations.
// We have two different epsilons to compensate for numerical side effects that occur
// around integers (e.g. 0, 1), and one epsilon to allow user error in trace input (SuperRelaxed).
extern const double epsilon;
extern const double epsilonRelaxed;
extern const double epsilonSuperRelaxed;


/*****************************************************************************
 *                           FUNCTION DECLARATIONS                           *
 *****************************************************************************/
inline double sqr(const double& src);
inline ValType sqr(const ValType& src);

// Returns whether num is zero up to epsilonRelaxed accuracy.
inline bool isZero(const double& num);
inline bool isZero(const ValType& num);
inline bool isZeroPermissive(const ValType& num);

// Returns whether num is one up to epsilonRelaxed accuracy.
inline bool isOne(const double& num);
inline bool isOne(const ValType& num);
inline bool isOnePermissive(const ValType& num);

// Returns whether the two values are equal up to epsilon accuracy.
inline bool isEqual(const double& lhs, const double& rhs);
inline bool isEqual(const ValType& lhs, const ValType& rhs);

bool operator<(const ValType& lhs, const ValType& rhs);

VectorXcd tensorProduct(const VectorXcd& lhs, const VectorXcd& rhs);
ValType trace(const MatrixXcd& mat);
ValType trace(const MatrixXcd& lhs, const MatrixXcd& rhs);

/*****************************************************************************
 *                          FUNCTION IMPLEMENTATIONS                         *
 *****************************************************************************/
double sqr(const double& src) {
	return pow(src, 2);
}

ValType sqr(const ValType& src) {
	return pow(src, 2);
}

bool isZero(const double& num) {
	return (abs(num) < epsilonRelaxed);
}

bool isZero(const ValType& num) {
	return (abs(num) < epsilonRelaxed);
}

bool isZeroPermissive(const ValType& num) {
	return (abs(num) < epsilonSuperRelaxed);
}

bool isOne(const double& num) {
	return (abs(num - 1.) < epsilonRelaxed);
}

bool isOne(const ValType& num) {
	return (abs(num - 1.) < epsilonRelaxed);
}


bool isOnePermissive(const ValType& num) {
	return (abs(num - 1.) < epsilonSuperRelaxed);
}

bool isEqual(const double& lhs, const double& rhs) {
	return (abs(rhs-lhs) < epsilon);
}

bool isEqual(const ValType& lhs, const ValType& rhs) {
	return (abs(rhs-lhs) < epsilon);
}

#endif // N_MATH_UTILS_H
