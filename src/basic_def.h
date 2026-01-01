#ifndef BASIC_DEF_H
#define BASIC_DEF_H

/*****************************************************************************
 *                             BUILD-TYPE OPTIONS                            *
 *****************************************************************************/

#ifdef __NRUNONSERVER__
// # if !defined(linux)
// #  error Server version is currently supported on Linux platforms only.
// # endif // not linux
#endif  // __NRUNONSERVER__


/*****************************************************************************
 *                       DEBUGGING AND ANALYSIS OPTIONS                      *
 *****************************************************************************/
#ifdef __NDEBUG__
#ifdef __NRANDOMSEED__  // define this to force a random seed even when building a debug version
                        // it takes precedence over __NNORANDOM__
#ifdef __NNORANDOM__
#undef __NNORANDOM__
#endif
#else  // don't use a random seed
#ifndef __NNORANDOM__
#define __NNORANDOM__
#endif  // __NNORANDOM__
#endif  // __NRANDOMSEED__

#ifndef __NSTRICT__  // define this for strict error checking - adds overhead
#define __NSTRICT__
#endif  // __NSTRICT__

#ifndef __NLOGALL__  // define this for logging all extra information available
#define __NLOGALL__
#endif  // __NLOGALL__

#ifndef __NCOLLECTSTATS__  // define this to collect runtime statistics
#define __NCOLLECTSTATS__
#endif  // __NCOLLECTSTATS__
#endif  // __NDEBUG__


/*****************************************************************************
 *                           MULTI-THREADED OPTIONS                          *
 *****************************************************************************/
#ifdef __NFULLMT__
#ifndef __N_MT_ADDSTATE__  // define this to add a new state using MT
#define __N_MT_ADDSTATE__
#endif  // __N_MT_ADDSTATE__

#ifndef __N_MT_BUILDMATRIX__  // define this to use build the mixed-state matrix using MT
#define __N_MT_BUILDMATRIX__
#endif  // __N_MT_BUILDMATRIX__
#endif  // __NFULLMT__


/*****************************************************************************
 *                            COMMON INCLUDE FILES                           *
 *****************************************************************************/
#include <complex>
#include <vector>
#include <Eigen/Core>

#define BASIC_DEF_INCLUDES
#include "logger.h"
#include "NError.h"
#include "NCollector.h"
#undef BASIC_DEF_INCLUDES

using Eigen::MatrixXcd;
using Eigen::MatrixXd;
using Eigen::VectorXcd;
using Eigen::VectorXd;
using std::complex;
using std::vector;

typedef unsigned int uint;
typedef complex<double> ValType;

#endif  // BASIC_DEF_H
