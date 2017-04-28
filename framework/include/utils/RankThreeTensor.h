/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RANKTHREETENSOR_H
#define RANKTHREETENSOR_H

// MOOSE includes
#include "DataIO.h"

// libMesh includes
#include "libmesh/tensor_value.h"
#include "libmesh/libmesh.h"
#include "libmesh/vector_value.h"

// Forward declarations
class MooseEnum;
class RankTwoTensor;
class RankThreeTensor;

template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankThreeTensor>(RankThreeTensor & v);

/**
 * RankThreeTensor is designed to handle any N-dimensional third order tensor, r.
 *
 */
class RankThreeTensor
{
public:
  /// Initialization method
  enum InitMethod
  {
    initNone
  };

  /**
   * To fill up the 27 entries in the 3rd-order tensor, fillFromInputVector
   * is called with one of the following fill_methods.
   * See the fill*FromInputVector functions for more details
   */
  enum FillMethod
  {
    general
  };

  /// Default constructor; fills to zero
  RankThreeTensor();

  /// Select specific initialization pattern
  RankThreeTensor(const InitMethod);

  /// Fill from vector
  RankThreeTensor(const std::vector<Real> &, FillMethod);

  /// Gets the value for the index specified.  Takes index = 0,1,2
  Real & operator()(unsigned int i, unsigned int j, unsigned int k);

  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   * used for const
   */
  Real operator()(unsigned int i, unsigned int j, unsigned int k) const;

  /// Zeros out the tensor.
  void zero();

  /// Print the rank three tensor
  void print(std::ostream & stm = Moose::out) const;

  /// copies values from a into this tensor
  RankThreeTensor & operator=(const RankThreeTensor & a);

  /// r_ijk*a_kl
  // RankTwoTensor operator*(const RankTwoTensor & a) const;

  /// r_ijk*a_kl
  // RealTensorValue operator*(const RealTensorValue & a) const;

  /// r_ijk*a
  RankThreeTensor operator*(const Real a) const;

  /// r_ijk *= a
  RankThreeTensor & operator*=(const Real a);

  /// r_ijk/a
  RankThreeTensor operator/(const Real a) const;

  /// r_ijk /= a  for all i, j, k
  RankThreeTensor & operator/=(const Real a);

  /// r_ijk += a_ijk  for all i, j, k
  RankThreeTensor & operator+=(const RankThreeTensor & a);

  /// r_ijkl + a_ijk
  RankThreeTensor operator+(const RankThreeTensor & a) const;

  /// r_ijk -= a_ijk
  RankThreeTensor & operator-=(const RankThreeTensor & a);

  /// r_ijk - a_ijk
  RankThreeTensor operator-(const RankThreeTensor & a) const;

  /// -r_ijk
  RankThreeTensor operator-() const;

  /// \sqrt(r_ijk*r_ijk)
  Real L2norm() const;

  /**
   * Rotate the tensor using
   * r_ijk = R_im R_in R_ko r_mno
   */
  template <class T>
  void rotate(const T & R);

  /**
   * Rotate the tensor using
   * r_ijk = R_im R_in R_ko r_mno
   */
  void rotate(const RealTensorValue & R);

  /**
   * Rotate the tensor using
   * r_ijk = R_im R_in R_ko R_lp r_mno
   */
  void rotate(const RankTwoTensor & R);

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

  /**
   * fillFromInputVector takes some number of inputs to fill
   * the Rank-3 tensor.
   * @param input the numbers that will be placed in the tensor
   * @param fill_method this can be:
   *             general (use fillGeneralFromInputVector)
   *             more fill_methods to be implemented soon!
   */
  void fillFromInputVector(const std::vector<Real> & input, FillMethod fill_method);

  /// rank three permutation tensor (also known as Levi-Civita symbol):
  /* eps(i1, i2, ..., iD) =
  *   +1 if (i1, i2, ..., iD) is an even permutation of (0, 1, ..., D)
  *   -1 if (i1, i2, ..., iD) is an odd permutation of (0, 1, ..., D)
  /*    0 otherwise */
  static int eps(unsigned int i, unsigned int j, unsigned int k);


protected:
  /// Dimensionality of rank-three tensor
  static const unsigned int N = LIBMESH_DIM;

  /// The values of the rank-three tensor
  Real _vals[N][N][N];

  void fillGeneralFromInputVector(const std::vector<Real> & input);

  template <class T>
  friend void dataStore(std::ostream &, T &, void *);

  template <class T>
  friend void dataLoad(std::istream &, T &, void *);
};

template <>
void dataStore(std::ostream &, RankThreeTensor &, void *);

template <>
void dataLoad(std::istream &, RankThreeTensor &, void *);

inline RankThreeTensor operator*(Real a, const RankThreeTensor & b) { return b * a; }

template <class T>
void
RankThreeTensor::rotate(const T & R)
{
  RankThreeTensor old = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
      {
        Real sum = 0.0;
        for (unsigned int m = 0; m < N; ++m)
          for (unsigned int n = 0; n < N; ++n)
            for (unsigned int o = 0; o < N; ++o)
              sum += R(i, m) * R(j, n) * R(k, o) * old(m, n, o);

        _vals[i][j][k] = sum;
      }
}

#endif // RANKTHREETENSOR_H
