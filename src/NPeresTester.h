#include "NMathUtils.h"


/*****************************************************************************
 *                              Class NPeresTester                             *
 *****************************************************************************/
class NPeresTester {
public:
	NPeresTester(void);

	/*****************************************************************************
	 *                          PUBLIC MEMBER FUNCTIONS                          *
	 *****************************************************************************/
	VectorXcd peresTest(const MatrixXcd& src, const vector<uint>& particleSizes) const;


	~NPeresTester(void);

private:
	uint multVec(const vector<uint>& vec, uint start, uint end) const;
};
