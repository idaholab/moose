#ifndef ELASTICITYTENSOR_H
#define ELASTICITYTENSOR_H

#include "ColumnMajorMatrix.h"

/**
 * This class defines a basic set of capabilities any elasticity tensor should have.
 *
 * The ElasticityTensor (also called C_ijkl) is a 3x3x3x3 tensor that is represented
 * here by a 9x9 matrix.
 *
 * Note that there is a pure virtual function: calculateEntries.  This function
 * MUST be ovewritten by derived classes!
 */
class ElasticityTensor : public ColumnMajorMatrix
{
public:
  /**
   * Default constructor... creates a 9x9 matrix.
   *
   * @param constant Determines whether or not the matrix will get recomputed
   * multiple times (false) or just once (true).
   */
  ElasticityTensor(const bool constant = false);

  /**
   * Public function that will be called whenever the values for this matrix
   * need to be filled in.
   */
  void calculate(unsigned int qp);
  
  virtual ColumnMajorMatrix calculateDerivative(unsigned int qp,unsigned int i){}

protected:

  /**
   * Whether or not the matrix is constant for all of time and space.
   */
  bool _constant;

  /**
   * Whether or not the values have been computed once.
   */
  bool _values_computed;

  /**
   * Pure virtual (must be overriden by derived class).
   *
   * This method actually fills in the entries of the tensor... using whatever
   * information it has.
   */
  virtual void calculateEntries(unsigned int qp) = 0;
};



#endif //ELASTICITYTENSOR_H
