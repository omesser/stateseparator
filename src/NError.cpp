#include "NError.h"

string NError::errorStrings[] = {
    "",                                                            // RES_SUCCESS
    "",                                                            // RES_UNKNOWN_ERROR
    "",                                                            // RES_INTERNAL_ERROR
    "Number of argument passed to main does not match expected.",  // RES_INVALID_ARGS_NUMBER
    "Invalid particle sizes (should be postive integers only).",   // RES_INVALID_SIZES
    "State size exceeds the maximum allowed size.",                // RES_STATE_TOO_BIG
    "Invalid matrix value found (should be decimal complex numbers).\n"
    "Hint: make sure there are no extra spaces near parenthesis and commas.",    // RES_INVALID_MATRIX_VAL
    "Not enough matrix rows were passed.",                                       // RES_MATRIX_TOO_SMALL
    "Too many matrix rows were passed.",                                         // RES_MATRIX_TOO_BIG
    "Matrix size does not match the Qudits dimensions.",                         // RES_ROW_SIZE_MISMATCH
    "The trace of the given matrix does not equal 1.",                           // RES_MATRIX_TRACE_NOT_ONE
    "The given matrix is not hermitian.",                                        // RES_MATRIX_NOT_HERMITIAN
    "The given matrix is not semi-definite positive.",                           // RES_MATRIX_NOT_POSITIVE
    "Invalid distance (should be a positive real number).",                      // RES_INVALID_DISTANCE
    "Invalid minimum probability (should be a positive real, smaller than 1).",  // RES_INVALID_PROB
    "Invalid number of states (should be a positive integer).",                  // RES_INVALID_NUM_OF_STATES
    "Encountered a string parse error.",                                         // RES_STRING_ERROR
    "Invalid output precision.",                                                 // RES_INVALID_PRECISION
};
