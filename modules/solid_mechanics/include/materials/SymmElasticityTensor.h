#ifndef SYMMELASTICITYTENSOR_H
#define SYMMELASTICITYTENSOR_H

#include "SymmTensor.h"
#include "vector_value.h"

/**
 * This class defines a basic set of capabilities any elasticity tensor should have.
 *
 * The SymmElasticityTensor represents the C_ijkl 3x3x3x3 tensor by a symmetric
 * 6x6 matrix.
 *
 */
class SymmElasticityTensor
{
public:
  /**
   * Default constructor...
   *
   * @param constant Determines whether or not the matrix will get recomputed
   * multiple times (false) or just once (true).
   */
  SymmElasticityTensor(const bool constant = false);

  /**
   * Public function that will be called whenever the values for this matrix
   * need to be filled in.
   */
  void calculate(unsigned int qp);

  virtual ColumnMajorMatrix calculateDerivative(unsigned int qp, unsigned int i);

  virtual void multiply( const SymmTensor & x, SymmTensor & b );

  virtual Real stiffness( const unsigned i, const unsigned j,
                          const RealGradient & test,
                          const RealGradient & phi );

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

  void generalMultiply( const SymmTensor & x, SymmTensor & b );

  Real _val[21]; // 6 in first row (column)
                 // 5 in second
                 // 4 in third
                 // 3 in fourth
                 // 2 in fifth
                 // 1 in sixth
};



#endif //SYMMELASTICITYTENSOR_H
