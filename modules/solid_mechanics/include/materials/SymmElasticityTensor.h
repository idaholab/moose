/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SYMMELASTICITYTENSOR_H
#define SYMMELASTICITYTENSOR_H

#include "SymmTensor.h"

#include "MaterialProperty.h"

#include "libmesh/vector_value.h"

#include <vector>

/**
 * This class defines a basic set of capabilities any elasticity tensor should have.
 *
 * The SymmElasticityTensor represents the C_ijkl 3x3x3x3 tensor by a symmetric
 * 6x6 matrix.  21 entries are stored.
 *
 *   Entries:                  Indices:
 *   C11 C12 C13 C14 C15 C16     0  1  2  3  4  5
 *       C22 C23 C24 C25 C26        6  7  8  9 10
 *           C33 C34 C35 C36          11 12 13 14
 *               C44 C45 C46             15 16 17
 *                   C55 C56                18 19
 *                       C66                   20
 *
 *   Multiplication by the strain is done by:
 *  |C11 C12 C13 C14 C15 C16| |  exx|   |Sxx|
 *  |    C22 C23 C24 C25 C26| |  eyy|   |Syy|
 *  |        C33 C34 C35 C36| |  ezz| = |Szz|
 *  |            C44 C45 C46| |2*exy|   |Sxy|
 *  |                C55 C56| |2*eyz|   |Syz|
 *  |                    C66| |2*ezx|   |Szx|
 *
 *   If the elasticity matrix is taken as 9x9 (with a column-major
 *     representation of the stress/strain), the relationship is:
 *   C11 C14 C16 C14 C12 C15 C16 C15 C13
 *       C44 C46 C44 C42 C45 C46 C45 C43
 *           C66 C64 C62 C65 C66 C65 C63
 *               C44 C42 C45 C46 C45 C43
 *                   C22 C25 C26 C25 C23
 *                       C55 C56 C55 C53
 *                           C66 C65 C63
 *                               C55 C53
 *                                   C33
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

  void constant(bool c) { _constant = c; }

  virtual ~SymmElasticityTensor() {}

  void copyValues(SymmElasticityTensor & rhs) const
  {
    for (unsigned i(0); i < 21; ++i)
    {
      rhs._val[i] = _val[i];
    }
  }

  /**
   * Public function that will be called whenever the values for this matrix
   * need to be filled in.
   */
  void calculate(unsigned int qp);

  virtual void multiply(const SymmTensor & x, SymmTensor & b) const;
  SymmTensor operator*(const SymmTensor & x) const;
  SymmElasticityTensor operator*(Real x) const;

  virtual Real stiffness(const unsigned int i,
                         const unsigned int j,
                         const RealGradient & test,
                         const RealGradient & phi) const;

  SymmElasticityTensor operator+(const SymmElasticityTensor & rhs) const
  {
    SymmElasticityTensor t = *this;

    t += rhs;

    return t;
  }

  void operator+=(const SymmElasticityTensor & rhs)
  {
    for (unsigned i(0); i < 21; ++i)
    {
      _val[i] += rhs._val[i];
    }
  }

  void operator-=(const SymmElasticityTensor & rhs)
  {
    for (unsigned i(0); i < 21; ++i)
    {
      _val[i] -= rhs._val[i];
    }
  }

  void operator*=(Real rhs)
  {
    for (unsigned i(0); i < 21; ++i)
    {
      _val[i] *= rhs;
    }
  }

  void operator/=(Real rhs)
  {
    for (unsigned i(0); i < 21; ++i)
    {
      _val[i] /= rhs;
    }
  }

  void zero()
  {
    for (unsigned i(0); i < 21; ++i)
    {
      _val[i] = 0;
    }
  }

  void convertFrom9x9(const ColumnMajorMatrix & cmm);
  void convertFrom6x6(const ColumnMajorMatrix & cmm);

  ColumnMajorMatrix columnMajorMatrix9x9() const;
  ColumnMajorMatrix columnMajorMatrix6x6() const;

  void form9x9Rotation(const ColumnMajorMatrix & R_3x3, ColumnMajorMatrix & R_9x9) const;
  void rotateFromGlobalToLocal(const ColumnMajorMatrix & R);
  void rotateFromLocalToGlobal(const ColumnMajorMatrix & R);

  virtual void adjustForCracking(const RealVectorValue & crack_flags);
  virtual void adjustForCrackingWithShearRetention(const RealVectorValue & crack_flags);

  virtual SymmElasticityTensor calculateDerivative(unsigned int qp, unsigned int i);

  friend std::ostream & operator<<(std::ostream & stream, const SymmElasticityTensor & obj);

  void fillFromInputVector(std::vector<Real> input, bool all);

  Real sum_3x3() const;
  RealGradient sum_3x1() const;

  /*
   * @return the value of the tensor given the index supplied.
   */
  Real valueAtIndex(int i) const;

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
   * Virtual (must be overriden by derived class).
   *
   * This method actually fills in the entries of the tensor... using whatever
   * information it has.
   */
  virtual void calculateEntries(unsigned int qp);

  Real _val[21]; // 6 in first row (column)
                 // 5 in second
                 // 4 in third
                 // 3 in fourth
                 // 2 in fifth
                 // 1 in sixth

  template <class T>
  friend void dataStore(std::ostream &, T &, void *);

  template <class T>
  friend void dataLoad(std::istream &, T &, void *);
};

template <>
void dataStore(std::ostream &, SymmElasticityTensor &, void *);

template <>
void dataLoad(std::istream &, SymmElasticityTensor &, void *);

#endif // SYMMELASTICITYTENSOR_H
