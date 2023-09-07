//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"
#include "ADRankThreeTensorForward.h"
#include "ADSymmetricRankTwoTensorForward.h"
#include "MooseUtils.h"
#include "MathUtils.h"

// Any requisite includes here
#include "libmesh/libmesh.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"
#include "libmesh/int_range.h"

#include "metaphysicl/raw_type.h"

#include <petscsys.h>
#include <vector>

// Forward declarations
class MooseEnum;
template <typename T>
class MooseArray;
typedef MooseArray<Real> VariableValue;
template <typename>
class ColumnMajorMatrixTempl;
namespace libMesh
{
template <typename>
class TypeVector;
template <typename>
class TypeTensor;
template <typename>
class TensorValue;
}

namespace MathUtils
{
template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankTwoTensor>(RankTwoTensor & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<ADRankTwoTensor>(ADRankTwoTensor & v);
}

/**
 * RankTwoTensorTempl is designed to handle the Stress or Strain Tensor for a fully anisotropic
 * material. It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M. R. Tonks
 *
 * RankTwoTensorTempl holds the 9 separate Sigma_ij or Epsilon_ij entries.
 * The entries are accessed by index, with i, j equal to 1, 2, or 3, or
 * internally i, j = 0, 1, 2.
 */
template <typename T>
class RankTwoTensorTempl : public libMesh::TensorValue<T>
{
public:
  /// @{

  /**
   * @brief Tensor dimension, i.e. number of rows/columns of the second order tensor
   */
  static constexpr unsigned int N = Moose::dim;

  /**
   * @brief The square of the tensor dimension.
   */
  static constexpr unsigned int N2 = N * N;

  /**
   * @brief The initialization method.
   * @see fillFromInputVector()
   * @see RankTwoTensorTempl(const std::vector<T> &)
   */
  enum InitMethod
  {
    initNone,
    initIdentity
  };

  /**
   * To fill up the 9 entries in the 2nd-order tensor, fillFromInputVector
   * is called with one of the following fill_methods.
   * See the fill*FromInputVector functions for more details
   */
  enum FillMethod
  {
    autodetect = 0,
    isotropic1 = 1,
    diagonal3 = 3,
    symmetric6 = 6,
    general = 9
  };

  /**
   * @brief Initialize the random seed based on an unsigned integer.
   * Deprecated in favor of MooseRandom::seed().
   */
  static void initRandom(unsigned int);

  /**
   * @brief Get the available `FillMethod` options.
   *
   * This method is useful in validParams().
   */
  [[nodiscard]] static MooseEnum fillMethodEnum();

  /**
   * @brief Fill a `libMesh::TensorValue<T>` from this second order tensor
   */
  void fillRealTensor(libMesh::TensorValue<T> &);

  /// Print the rank two tensor
  void print(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the RankTwoTensorTempl<ADReal>
  void printReal(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the RankTwoTensorTempl<ADReal> along with its first nDual dual numbers
  void printDualReal(unsigned int nDual, std::ostream & stm = Moose::out) const;

  /// @}

  /// @{

  /**
   * @brief Empty constructor; fills to zero
   *
   * \f$ A_{ij} = 0 \f$
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl();

  /**
   * @brief Select specific initialization pattern.
   *
   * `initNone` initializes an empty second order tensor.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(RankTwoTensor::initNone);
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * `initIdentity` initializes a second order identity tensor.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(RankTwoTensor::initIdentity);
   * // A = [ 1 0 0
   * //       0 1 0
   * //       0 0 1 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl(const InitMethod);

  /**
   * @brief Initialize from row vectors.
   * @see initializeFromRows()
   *
   * Deprecated in favor of initializeFromRows()
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue row1(1, 2, 3);
   * RealVectorValue row2(4, 5, 6);
   * RealVectorValue row3(7, 8, 9);
   * RankTwoTensor A(row1, row2, row3);
   * // A = [ 1 2 3
   * //       4 5 6
   * //       7 8 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl(const libMesh::TypeVector<T> & row1,
                     const libMesh::TypeVector<T> & row2,
                     const libMesh::TypeVector<T> & row3);

  /**
   * @brief Constructor that proxies the fillFromInputVector() method
   * @see fillFromInputVector()
   */
  RankTwoTensorTempl(const std::vector<T> & input) { this->fillFromInputVector(input); };

  /**
   * @brief Initialize a symmetric second order tensor using the 6 arguments.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6);
   * // A = [ 1 6 5
   * //       6 2 4
   * //       5 4 3 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl(
      const T & S11, const T & S22, const T & S33, const T & S23, const T & S13, const T & S12);

  /**
   * @brief Initialize a second order tensor using the 9 arguments in a column-major fashion.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl(const T & S11,
                     const T & S21,
                     const T & S31,
                     const T & S12,
                     const T & S22,
                     const T & S32,
                     const T & S13,
                     const T & S23,
                     const T & S33);

  /**
   * @brief The copy constructor
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * RankTwoTensor B = A;
   * // A = B = [ 1 4 7
   * //           2 5 8
   * //           3 6 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl(const RankTwoTensorTempl<T> & a) = default;

  /**
   * @brief The conversion operator from a `libMesh::TensorValue`
   */
  RankTwoTensorTempl(const libMesh::TensorValue<T> & a) : libMesh::TensorValue<T>(a) {}

  /**
   * @brief The conversion operator from a `libMesh::TypeTensor`
   */
  RankTwoTensorTempl(const libMesh::TypeTensor<T> & a) : libMesh::TensorValue<T>(a) {}

  /**
   * @brief The conversion operator from a `SymmetricRankTwoTensorTempl`
   */
  template <typename T2>
  RankTwoTensorTempl(const SymmetricRankTwoTensorTempl<T2> & a)
    : RankTwoTensorTempl<T>(a(0),
                            a(1),
                            a(2),
                            a(3) / MathUtils::sqrt2,
                            a(4) / MathUtils::sqrt2,
                            a(5) / MathUtils::sqrt2)
  {
  }

  /**
   * @brief The conversion operator from `RankTwoTensorTempl<T2>` to `RankTwoTensorTempl<T>` where
   * `T2` is convertible to `T`.
   */
  template <typename T2>
  RankTwoTensorTempl(const RankTwoTensorTempl<T2> & a) : libMesh::TensorValue<T>(a)
  {
  }
  /// @} end Constructor

  /// @{

  /**
   * @brief Named constructor for initializing symmetrically.
   *
   * The supplied vectors are used as row and column vectors to construct two tensors respectively,
   * that are averaged to create a symmetric tensor.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue row1(1, 2, 3);
   * RealVectorValue row2(4, 5, 6);
   * RealVectorValue row3(7, 8, 9);
   * auto A = RankTwoTensor::initializeSymmetric(row1, row2, row3);
   * // A = [ 1 3 5
   * //       3 5 7
   * //       5 7 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl initializeSymmetric(const libMesh::TypeVector<T> & v0,
                                                              const libMesh::TypeVector<T> & v1,
                                                              const libMesh::TypeVector<T> & v2);

  /**
   * @brief Named constructor for initializing from row vectors.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue row1(1, 2, 3);
   * RealVectorValue row2(4, 5, 6);
   * RealVectorValue row3(7, 8, 9);
   * auto A = RankTwoTensor::initializeFromRows(row1, row2, row3);
   * // A = [ 1 2 3
   * //       4 5 6
   * //       7 8 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl initializeFromRows(const libMesh::TypeVector<T> & row0,
                                                             const libMesh::TypeVector<T> & row1,
                                                             const libMesh::TypeVector<T> & row2);

  /**
   * @brief Named constructor for initializing from row vectors.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue col1(1, 2, 3);
   * RealVectorValue col2(4, 5, 6);
   * RealVectorValue col3(7, 8, 9);
   * auto A = RankTwoTensor::initializeFromColumns(col1, col2, col3);
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl
  initializeFromColumns(const libMesh::TypeVector<T> & col0,
                        const libMesh::TypeVector<T> & col1,
                        const libMesh::TypeVector<T> & col2);

  /**
   * @brief Initialize a second order identity tensor.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * auto A = RankTwoTensor::Identity();
   * // A = [ 1 0 0
   * //       0 1 0
   * //       0 0 1 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl Identity() { return RankTwoTensorTempl(initIdentity); }

  /**
   * @brief Initialize a second order tensor with expression \f$ B_{ij} = F_{ij} F_{ji} \f$.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor F(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // F = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * auto B = RankTwoTensor::timesTranspose(F);
   * // B = [ 66 78  90
   * //       78 93  108
   * //       90 108 126 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl<T> timesTranspose(const RankTwoTensorTempl<T> &);

  /**
   * @brief Initialize a second order tensor with expression \f$ C_{ij} = F_{ji} F_{ij} \f$.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor F(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // F = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * auto C = RankTwoTensor::transposeTimes(F);
   * // C = [ 14 32  50
   * //       32 77  122
   * //       50 122 194 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl<T> transposeTimes(const RankTwoTensorTempl<T> &);

  /**
   * @brief Initialize a second order tensor with expression \f$ E_{ij} = C_{ij} + C_{ji} \f$.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor C(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // C = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * auto E = RankTwoTensor::plusTranspose(C);
   * // E = [ 2  6  10
   * //       6  10 14
   * //       10 14 18 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl<T> plusTranspose(const RankTwoTensorTempl<T> &);

  /**
   * @brief Initialize a second order tensor as the outer product of two vectors, i.e. \f$ A_{ij} =
   * a_i b_j \f$.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue a(1, 2, 3);
   * RealVectorValue b(4, 5, 6);
   * auto A = RankTwoTensor::outerProduct(a, b);
   * // A = [ 4  5  6
   * //       8  10 12
   * //       12 15 18 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl<T> outerProduct(const libMesh::TypeVector<T> &,
                                                          const libMesh::TypeVector<T> &);

  /**
   * @brief Initialize a second order tensor as the outer product of a vector with itself, i.e. \f$
   * A_{ij} = a_i a_j \f$.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue a(1, 2, 3);
   * auto A = RankTwoTensor::selfOuterProduct(a);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  [[nodiscard]] static RankTwoTensorTempl<T> selfOuterProduct(const libMesh::TypeVector<T> &);

  /**
   * @brief Generate a random second order tensor with all 9 components treated as independent
   * random variables following a Gaussian distribution.
   *
   * @param stddev The standard deviation of the Gaussian distribution
   * @param mean   The mean of the Gaussian distribution
   */
  [[nodiscard]] static RankTwoTensorTempl<T> genRandomTensor(T stddev, T mean);

  /**
   * @brief Generate a random symmetric second order tensor with the 6 upper-triangular components
   * treated as independent random variables following a Gaussian distribution.
   *
   * @param stddev The standard deviation of the Gaussian distribution
   * @param mean   The mean of the Gaussian distribution
   */
  [[nodiscard]] static RankTwoTensorTempl<T> genRandomSymmTensor(T stddev, T mean);

  /// @}

  /// @{

  /**
   * @brief Get the i-th column of the second order tensor.
   *
   * @param i The column number, i = 0, 1, 2
   * @return The i-th column of the second order tensor.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * RealVectorValue col = A.column(1);
   * // col = [ 4
   * //         5
   * //         6 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  VectorValue<T> column(const unsigned int i) const;

  /// @}

  /// @{

  /**
   * @brief Assignment operator
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * A = B;
   * // A = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> & operator=(const RankTwoTensorTempl<T> & a);

  /**
   * @brief Assignment operator (from a ColumnMajorMatrixTempl<T>)
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * RealVectorValue col1(1, 2, 3);
   * RealVectorValue col2(4, 5, 6);
   * RealVectorValue col3(7, 8, 9);
   * ColumnMajorMatrix B(col1, col2, col3);
   * // B = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * A = B;
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> & operator=(const ColumnMajorMatrixTempl<T> & a);

  /**
   * @brief Assignment-from-scalar operator.  Used only to zero out the tensor.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * A = 0;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * A = 1;
   * // This triggers an assertion failure.
   * A = 0.0;
   * // This triggers an assertion failure.
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<ScalarTraits<Scalar>::value, RankTwoTensorTempl &>::type
  operator=(const Scalar & libmesh_dbg_var(p))
  {
    libmesh_assert_equal_to(p, Scalar(0));
    this->zero();
    return *this;
  }

  /**
   * Add another second order tensor to this one
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * A += B;
   * // A = [ 10 8  6
   * //       10 9  8
   * //       10 10 10 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> & operator+=(const RankTwoTensorTempl<T> & a);

  /**
   * Subtract another second order tensor from this one
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * A -= B;
   * // A = [ -8 -4 0
   * //       -6 -1 4
   * //       -4  2 8 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> & operator-=(const RankTwoTensorTempl<T> & a);

  /**
   * Multiply this tensor by a scalar (component-wise)
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * A *= 2;
   * // A = [ 2 4  6
   * //       4 8  12
   * //       6 12 18 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> & operator*=(const T & a);

  /**
   * Divide this tensor by a scalar (component-wise)
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * A /= 2;
   * // A = [ 0.5 1.0 1.5
   * //       1.0 2.0 3.0
   * //       1.5 3.0 4.5 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> & operator/=(const T & a);

  /**
   * Multiplication with another second order tensor
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * A *= B;
   * // A = [ 90  54 18
   * //       114 69 24
   * //       138 84 30 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> & operator*=(const libMesh::TypeTensor<T> & a);

  /**
   * @brief The smart mutator that determines how to fill the second order tensor based on the size
   * of the input vector.
   *
   * @param input The input vector, can be of size 1, 3, 6, or 9
   * @param fill_method The fill method, default to autodetect.
   *
   * When `input.size() == 1`, the vector value is used to fill the diagonal components of the
   * second order tensor:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * A.fillFromInputVector({1.5});
   * // A = [ 1.5 0   0
   * //       0   1.5 0
   * //       0   0   1.5 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * When `input.size() == 3`, the vector values are used to fill the diagonal components of the
   * second order tensor:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * A.fillFromInputVector({1, 2, 3});
   * // A = [ 1 0 0
   * //       0 2 0
   * //       0 0 3 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * When `input.size() == 6`, the second order tensor is filled symmetrically using the Voigt
   * notation:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * A.fillFromInputVector({1, 2, 3, 4, 5, 6});
   * // A = [ 1 6 5
   * //       6 2 4
   * //       5 4 3 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * When `input.size() == 9`, all components of the second order tensor are filled in a
   * column-major fashion:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * A.fillFromInputVector({1, 2, 3, 4, 5, 6, 7, 8, 9});
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method = autodetect);

  /**
   * @brief The smart mutator that determines how to fill the second order tensor based on the order
   * of the scalar_variable.
   *
   * @param scalar_variable The input scalar variable. Supported orders are FIRST, THIRD, and SIXTH.
   *
   * When `scalar_variable.size() == 1`, the scalar value is used to fill the very first component
   * of the second order tensor:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   & // Suppose v[0] = 1
   * RankTwoTensor A;
   * A.fillFromScalarVariable(v);
   * // A = [ 1 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * When `scalar_variable.size() == 3`, the scalar values are used to fill the in-plane components
   * of the second order tensor using the Voigt notation:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * // Suppose v[0] = 1
   * //         v[1] = 2
   * //         v[2] = 3
   * RankTwoTensor A;
   * A.fillFromScalarVariable(v);
   * // A = [ 1 3 0
   * //       3 2 0
   * //       0 0 0 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * When `scalar_variable.size() == 6`, the second order tensor is filled symmetrically using the
   * Voigt notation:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * // Suppose v[0] = 1
   * //         v[1] = 2
   * //         v[2] = 3
   * //         v[3] = 4
   * //         v[4] = 5
   * //         v[5] = 6
   * RankTwoTensor A;
   * A.fillFromScalarVariable(v);
   * // A = [ 1 6 5
   * //       6 2 4
   * //       5 4 3 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void fillFromScalarVariable(const VariableValue & scalar_variable);

  /**
   * sets _coords[0][0], _coords[0][1], _coords[1][0], _coords[1][1] to input,
   * and the remainder to zero
   */
  void surfaceFillFromInputVector(const std::vector<T> & input);

  /**
   * @brief Set the values of the second order tensor to be the outer product of two vectors, i.e.
   * \f$ A_{ij} = a_i b_j \f$.
   *
   * Deprecated in favor of outerProduct()
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue a(1, 2, 3);
   * RealVectorValue b(4, 5, 6);
   * RankTwoTensor A;
   * A.vectorOuterProduct(a, b);
   * // A = [ 4  5  6
   * //       8  10 12
   * //       12 15 18 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void vectorOuterProduct(const libMesh::TypeVector<T> &, const libMesh::TypeVector<T> &);

  /**
   * @brief Assign values to a specific row of the second order tensor.
   *
   * @param r The row number, r = 0, 1, 2
   * @param v The values to be set
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue a(1, 2, 3);
   * RankTwoTensor A;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * A.fillRow(1, a);
   * // A = [ 0 0 0
   * //       1 2 3
   * //       0 0 0 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void fillRow(unsigned int r, const libMesh::TypeVector<T> & v);

  /**
   * @brief Assign values to a specific column of the second order tensor.
   *
   * @param c The column number, c = 0, 1, 2
   * @param v The values to be set
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RealVectorValue a(1, 2, 3);
   * RankTwoTensor A;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * A.fillColumn(1, a);
   * // A = [ 0 1 0
   * //       0 2 0
   * //       0 3 0 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void fillColumn(unsigned int c, const libMesh::TypeVector<T> & v);

  /**
   * @brief Set the tensor to identity.
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * A.setToIdentity();
   * // A = [ 1 0 0
   * //       0 1 0
   * //       0 0 1 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void setToIdentity();

  /// Add identity times a to _coords
  /**
   * @brief Add a scalar to diagonal components \f$ A_{ij} + a\delta_{ij} \f$
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A;
   * // A = [ 0 0 0
   * //       0 0 0
   * //       0 0 0 ]
   * A.addIa(1.5);
   * // A = [ 1.5 0   0
   * //       0   1.5 0
   * //       0   0   1.5 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void addIa(const T & a);

  /**
   * Rotate the tensor in-place given a rotation tensor
   * \f$ A_{ij} \leftarrow R_{ij} A_{jk} R_{jk} \f$
   * @param R The rotation tensor
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * RankTwoTensor R(0, 1, 0, 1, 0, 0, 0, 0, 1);
   * // R = [ 0 1 0
   * //       1 0 0
   * //       0 0 1 ]
   * A.rotate(R);
   * // A = [ 5 2 8
   * //       4 1 7
   * //       6 3 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  void rotate(const RankTwoTensorTempl<T> & R);

  /// @}

  /// @{

  /**
   * @brief Return \f$ A_{ij} = A_{ik}A_{kj} \f$
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * RankTwoTensor B = A.square();
   * // B = [ 30 66 102
   * //       36 81 126
   * //       42 96 150 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> square() const;

  /**
   * Return the rotated tensor given a rotation tensor
   * \f$ A'_{ij} = R_{ij} A_{jk} R_{jk} \f$
   * @param R The rotation tensor
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * RankTwoTensor R(0, 1, 0, 1, 0, 0, 0, 0, 1);
   * // R = [ 0 1 0
   * //       1 0 0
   * //       0 0 1 ]
   * RankTwoTensor A_rotated = A.rotated(R);
   * // A_rotated = [ 5 2 8
   * //               4 1 7
   * //               6 3 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> rotated(const RankTwoTensorTempl<T> & R) const;

  /**
   * Rotate the tensor about the z-axis
   * @param a The rotation angle in radians
   */
  RankTwoTensorTempl<T> rotateXyPlane(T a);

  /**
   * Return the tensor transposed
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 4 7
   * //       2 5 8
   * //       3 6 9 ]
   * RankTwoTensor At = A.transpose();
   * // At = [ 1 2 3
   * //        4 5 6
   * //        7 8 9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> transpose() const;

  /**
   * Return the sum of two second order tensors
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * RankTwoTensor C = A + B;
   * // C = [ 10 8  6
   * //       10 9  8
   * //       10 10 10 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  template <typename T2>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator+(const libMesh::TypeTensor<T2> & a) const;

  /**
   * Return the subtraction of two second order tensors
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * RankTwoTensor C = A - B;
   * // C = [ -8 -4 0
   * //       -6 -1 4
   * //       -4  2 8 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  template <typename T2>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator-(const libMesh::TypeTensor<T2> & a) const;

  /**
   * Return the negation of this tensor
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B = -A;
   * // B = [ -1 -2 -3
   * //       -2 -4 -6
   * //       -3 -6 -9 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  RankTwoTensorTempl<T> operator-() const;

  /**
   * Return this tensor multiplied by a scalar (component-wise)
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B = A * 2;
   * // B = [ 2 4  6
   * //       4 8  12
   * //       6 12 18 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type = 0>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype> operator*(const T2 & a) const;

  /**
   * Return this tensor divided by a scalar (component-wise)
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B = A / 2;
   * // B = [ 0.5 1.0 1.5
   * //       1.0 2.0 3.0
   * //       1.5 3.0 4.5 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type = 0>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype> operator/(const T2 & a) const;

  /**
   * Return this tensor multiplied by a vector. \f$ b_i = A_{ij} a_j \f$
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RealVectorValue a(1, 2, 3);
   * // a = [ 1
   * //       2
   * //       3 ]
   * RealVectorValue b = A * a;
   * // b = [ 30
   * //       36
   * //       42 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  template <typename T2>
  libMesh::TypeVector<typename CompareTypes<T, T2>::supertype>
  operator*(const libMesh::TypeVector<T2> & a) const;

  /**
   * Multiplication with another second order tensor
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * RankTwoTensor C = A * B;
   * // C = [ 90  54 18
   * //       114 69 24
   * //       138 84 30 ]
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  template <typename T2>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator*(const libMesh::TypeTensor<T2> & a) const;

  /**
   * Return the double contraction with another second order tensor \f$ A_{ij} B_{ij} \f$
   *
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
   * // A = [ 1 2 3
   * //       2 4 6
   * //       3 6 9 ]
   * RankTwoTensor B(9, 8, 7, 6, 5, 4, 3, 2, 1);
   * // B = [ 9 6 3
   * //       8 5 2
   * //       7 4 1 ]
   * Real result = A.doubleContraction(B);
   * // result = 143
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */

  T doubleContraction(const RankTwoTensorTempl<T> & a) const;

  /**
   * Return the general tensor product of this second order tensor and another second order tensor
   * defined as \f$ C_{ijkl} = A_{\mathcal{M}(n)\mathcal{M}(o)} B_{\mathcal{M}(p)\mathcal{M}(q)} \f$
   * where the multiplication order is defined by the index map \f$ \mathcal{M}: \{n,o,p,q\} \to
   * \{i,j,k,l\} \f$. The index map is specified using the template parameters. See examples below
   * for detailed explanation.
   *
   * Suppose we have two second order tensors A and B, and we denote the output indices as i, j, k,
   * l:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * usingTensorIndices(i, j, k, l);
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * The outer product of A and B is defined as \f$ A_{ij} B_{kl} \f$, hence the template
   * specialization should be `times<i, j, k, l>`
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankFourTensor C = A.times<i, j, k, l>(B);
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * The tensor product of A and B, i.e. \f$ A_{ik} B_{jl} \f$ can be expressed using the template
   * specialization `times<i, k, j, l>`
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankFourTensor C = A.times<i, k, j, l>(B);
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * Similarly, another tensor product of A and B, i.e. \f$ A_{il} B_{jk} \f$ can be expressed using
   * the template specialization `times<i, l, j, k>`
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankFourTensor C = A.times<i, l, j, k>(B);
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * The combination goes on...
   */
  template <int n, int o, int p, int q>
  RankFourTensorTempl<T> times(const RankTwoTensorTempl<T> & b) const;

  /**
   * Return the single contraction of this second order tensor with a fourth order tensor defined as
   * \f$ C_{ijkl} = A_{\mathcal{M}(m)\mathcal{M}(n)}
   * B_{\mathcal{M}(p)\mathcal{M}(q)\mathcal{M}(r)\mathcal{M}(s)} \f$ where the multiplication order
   * is defined by the index map \f$ \mathcal{M}: \{m,n,p,q,r,s\} \to \{i,j,k,l\} \f$. The index map
   * is specified using the template parameters. See examples below for detailed explanation.
   *
   * Suppose we have a second order tensors A and a fourth order tensor B, and we denote the indices
   * (four output indices and a dummy index to be contracted) as i, j, k, l, m:
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * usingTensorIndices(i, j, k, l, m);
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * The single contraction of A and B defined as \f$ A_{im} B_{mjkl} \f$ can be expressed using the
   * template specialization `times<i, m, m, j, k, l>`
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankFourTensor C = A.times<i, m, m, j, k, l>(B);
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * Similarly, another single contraction of A and B, i.e. \f$ A_{m, i} A_{j, k, m, l} \f$ can be
   * expressed using the template specialization `times<m, i, j, k, m, l>`
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
   * RankFourTensor C = A.times<m, i, j, k, m, l>(B);
   * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * The combination goes on. Note that this method assumes exactly one repeated index.
   */
  template <int n, int o, int p, int q, int r, int s>
  RankFourTensorTempl<T> times(const RankFourTensorTempl<T> & b) const;

  /**
   * @brief Return the outer product \f$ C_{ijkl} = A_{ij} B_{kl} \f$
   */
  RankFourTensorTempl<T> outerProduct(const RankTwoTensorTempl<T> & b) const
  {
    usingTensorIndices(i_, j_, k_, l_);
    return times<i_, j_, k_, l_>(b);
  }

  /**
   * @brief Return the single contraction of this second order tensor with a third order tensor \f$
   * C_{ikl} = A_{ij} B_{jkl} \f$
   */
  RankThreeTensorTempl<T> contraction(const RankThreeTensorTempl<T> & b) const;

  /**
   * @brief Return the tensor product of this second order tensor with a vector \f$ C_{ijk} = A_{jk}
   * b_{i} \f$
   */
  RankThreeTensorTempl<T> mixedProductJkI(const VectorValue<T> & b) const;

  /**
   * @brief Return the positive projection tensor
   *
   * Consider the eigenvalue decomposition of this second order tensor \f$ A = V D V^T \f$, the part
   * of this tensor that lies on the positive spectrum is defined as \f$ A_+ = V \left<D\right> V^T
   * \f$ where the angled brackets are the Macaulay brackets. The positive projection tensor is the
   * linear projection from the full spectrum to the positive spectrum, i.e. \f$ A_+ = P A \f$. The
   * derivation of this positive projection tensor can be found in C. Miehe and M. Lambrecht,
   * Commun. Numer. Meth. Engng 2001; 17:337~353
   *
   * @param eigvals The three eigenvalues of this second order tensor will be filled into this
   * vector.
   * @param eigvecs The three eigenvectors of this second order tensor will be filled into this
   * tensor.
   * @return The fourth order positive projection tensor.
   */
  RankFourTensorTempl<T> positiveProjectionEigenDecomposition(std::vector<T> &,
                                                              RankTwoTensorTempl<T> &) const;

  /**
   * @brief Return the deviatoric part of this tensor \f$ A_{ij} - \frac{1}{3} A_{kk} \delta_{ij}
   * \f$
   */
  RankTwoTensorTempl<T> deviatoric() const;

  /**
   * @brief A wrapper for tr()
   * @see tr()
   */
  T trace() const;

  /**
   * Return the derivative of the trace w.r.t. this second order tensor itself \f$ \frac{\partial
   * A_{kk}}{\partial A_{ij}} = \delta_{ij} \f$
   */
  RankTwoTensorTempl<T> dtrace() const;

  /**
   * @brief Return the inverse of this second order tensor
   */
  RankTwoTensorTempl<T> inverse() const;

  /**
   * @brief Return the principal second invariant of this second order tensor
   *
   * \f$ I_2 = \frac{1}{2} \left( A_{kk}^2 - A_{ij}A_{ij} \right) \f$
   */
  T generalSecondInvariant() const;

  /**
   * @brief Return the main second invariant of this second order tensor
   *
   * \f$ J_2 = \frac{1}{2} \left( S_{ij}S_{ij} \right) \f$, where \f$ S_{ij} = A_{ij} -
   * \frac{1}{3}A_{kk}\delta_{ij} \f$
   */
  T secondInvariant() const;

  /**
   * Return the derivative of the main second invariant w.r.t. this second order tensor itself \f$
   * \frac{\partial J_2}{\partial A_{ij}} = S_{ij} = A_{ij} -
   * \frac{1}{3}A_{kk}\delta_{ij} \f$
   */
  RankTwoTensorTempl<T> dsecondInvariant() const;

  /**
   * Return the second derivative of the main second invariant w.r.t. this second order tensor
   * itself \f$ \frac{\partial^2 J_2}{\partial A_{ij}A_{kl}} = \delta_{ik}\delta_{jl} -
   * \frac{1}{3}\delta_{ij}\delta_{kl} \f$
   */
  RankFourTensorTempl<T> d2secondInvariant() const;

  /**
   * @brief Sin(3*Lode_angle)
   *
   * If secondInvariant() <= r0 then return r0_value
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  T sin3Lode(const T & r0, const T & r0_value) const;

  /**
   * d(sin3Lode)/dA_ij
   * If secondInvariant() <= r0 then return zero
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  RankTwoTensorTempl<T> dsin3Lode(const T & r0) const;

  /**
   * d^2(sin3Lode)/dA_ij/dA_kl
   * If secondInvariant() <= r0 then return zero
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  RankFourTensorTempl<T> d2sin3Lode(const T & r0) const;

  /**
   * Denote the _coords[i][j] by A_ij, then
   * S_ij = A_ij - de_ij*tr(A)/3
   * Then this returns det(S + S.transpose())/2
   * Note the explicit symmeterisation
   */
  T thirdInvariant() const;

  /**
   * Denote the _coords[i][j] by A_ij, then
   * this returns d(thirdInvariant()/dA_ij
   */
  RankTwoTensorTempl<T> dthirdInvariant() const;

  /**
   * Denote the _coords[i][j] by A_ij, then this returns
   * d^2(thirdInvariant)/dA_ij/dA_kl
   */
  RankFourTensorTempl<T> d2thirdInvariant() const;

  /**
   * Denote the _coords[i][j] by A_ij, then this returns
   * d(det)/dA_ij
   */
  RankTwoTensorTempl<T> ddet() const;

  /// Sqrt(_coords[i][j]*_coords[i][j])
  T L2norm() const;

  /**
   * computes eigenvalues, assuming tens is symmetric, and places them
   * in ascending order in eigvals
   */
  void symmetricEigenvalues(std::vector<T> & eigvals) const;

  /**
   * computes eigenvalues and eigenvectors, assuming tens is symmetric, and places them
   * in ascending order in eigvals.  eigvecs is a matrix with the first column
   * being the first eigenvector, the second column being the second, etc.
   */
  void symmetricEigenvaluesEigenvectors(std::vector<T> & eigvals,
                                        RankTwoTensorTempl<T> & eigvecs) const;

  /**
   * computes eigenvalues, and their symmetric derivatives wrt vals,
   * assuming tens is symmetric
   * @param eigvals are the eigenvalues of the matrix, in ascending order
   * @param deigvals Here digvals[i](j,k) = (1/2)*(d(eigvals[i])/dA_jk + d(eigvals[i]/dA_kj))
   * Note the explicit symmeterisation here.
   * For equal eigenvalues, these derivatives are not gauranteed to
   * be the ones you expect, since the derivatives in this case are
   * often defined by continuation from the un-equal case, and that is
   * too sophisticated for this routine.
   */
  void dsymmetricEigenvalues(std::vector<T> & eigvals,
                             std::vector<RankTwoTensorTempl<T>> & deigvals) const;

  /**
   * Computes second derivatives of Eigenvalues of a rank two tensor
   * @param deriv store second derivative of the current tensor in here
   */
  void d2symmetricEigenvalues(std::vector<RankFourTensorTempl<T>> & deriv) const;

  /**
   * Uses the petscblaslapack.h LAPACKsyev_ routine to perform RU decomposition and obtain the
   * rotation tensor.
   */
  void getRUDecompositionRotation(RankTwoTensorTempl<T> & rot) const;

  /// returns this_ij * b_ijkl
  RankTwoTensorTempl<T> initialContraction(const RankFourTensorTempl<T> & b) const;

  /// @}

  /// @{

  /// Defines logical equality with another RankTwoTensorTempl<T>
  bool operator==(const RankTwoTensorTempl<T> & a) const;

  /// Test for symmetry
  bool isSymmetric() const;

  /// @}

protected:
  /**
   * Uses the petscblaslapack.h LAPACKsyev_ routine to find, for symmetric _coords:
   *  (1) the eigenvalues (if calculation_type == "N")
   *  (2) the eigenvalues and eigenvectors (if calculation_type == "V")
   * @param calculation_type If "N" then calculation eigenvalues only
   * @param eigvals Eigenvalues are placed in this array, in ascending order
   * @param a Eigenvectors are placed in this array if calculation_type == "V".
   * See code in dsymmetricEigenvalues for extracting eigenvectors from the a output.
   */
  void syev(const char * calculation_type, std::vector<T> & eigvals, std::vector<T> & a) const;

private:
  static constexpr Real identityCoords[N2] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

  template <class T2>
  friend void dataStore(std::ostream &, RankTwoTensorTempl<T2> &, void *);

  using libMesh::TensorValue<T>::_coords;

  template <class T2>
  friend void dataLoad(std::istream &, RankTwoTensorTempl<T2> &, void *);
  template <class T2>
  friend class RankFourTensorTempl;
  template <class T2>
  friend class RankThreeTensorTempl;
};

namespace MetaPhysicL
{
template <typename T>
struct RawType<RankTwoTensorTempl<T>>
{
  typedef RankTwoTensorTempl<typename RawType<T>::value_type> value_type;

  static value_type value(const RankTwoTensorTempl<T> & in)
  {
    value_type ret;
    for (auto i : make_range(RankTwoTensorTempl<T>::N))
      for (auto j : make_range(RankTwoTensorTempl<T>::N))
        ret(i, j) = raw_value(in(i, j));

    return ret;
  }
};
}

template <typename T>
template <typename T2>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator+(const libMesh::TypeTensor<T2> & b) const
{
  return libMesh::TensorValue<T>::operator+(b);
}

template <typename T>
template <typename T2>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator-(const libMesh::TypeTensor<T2> & b) const
{
  return libMesh::TensorValue<T>::operator-(b);
}

template <typename T>
template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator*(const T2 & b) const
{
  return libMesh::TensorValue<T>::operator*(b);
}

template <typename T>
template <typename T2>
libMesh::TypeVector<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator*(const libMesh::TypeVector<T2> & b) const
{
  return libMesh::TensorValue<T>::operator*(b);
}

template <typename T>
template <typename T2>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator*(const libMesh::TypeTensor<T2> & b) const
{
  return libMesh::TensorValue<T>::operator*(b);
}

template <typename T>
template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator/(const T2 & b) const
{
  return libMesh::TensorValue<T>::operator/(b);
}

template <typename T>
RankFourTensorTempl<T>
RankTwoTensorTempl<T>::positiveProjectionEigenDecomposition(std::vector<T> & eigval,
                                                            RankTwoTensorTempl<T> & eigvec) const
{
  if constexpr (MooseUtils::IsLikeReal<T>::value)
  {
    // Compute eigenvectors and eigenvalues of this tensor
    this->symmetricEigenvaluesEigenvectors(eigval, eigvec);

    // Separate out positive and negative eigen values
    std::array<T, N> epos;
    std::array<T, N> d;
    for (auto i : make_range(N))
    {
      epos[i] = (std::abs(eigval[i]) + eigval[i]) / 2.0;
      d[i] = 0 < eigval[i] ? 1.0 : 0.0;
    }

    // projection tensor
    RankFourTensorTempl<T> proj_pos;
    RankFourTensorTempl<T> Gab, Gba;

    for (auto a : make_range(N))
    {
      const auto Ma = RankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(a));
      proj_pos += d[a] * Ma.outerProduct(Ma);
    }

    usingTensorIndices(i_, j_, k_, l_);
    for (const auto a : make_range(N))
      for (const auto b : make_range(a))
      {
        const auto Ma = RankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(a));
        const auto Mb = RankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(b));

        Gab = Ma.template times<i_, k_, j_, l_>(Mb) + Ma.template times<i_, l_, j_, k_>(Mb);
        Gba = Mb.template times<i_, k_, j_, l_>(Ma) + Mb.template times<i_, l_, j_, k_>(Ma);

        T theta_ab;
        if (!MooseUtils::absoluteFuzzyEqual(eigval[a], eigval[b]))
          theta_ab = 0.5 * (epos[a] - epos[b]) / (eigval[a] - eigval[b]);
        else
          theta_ab = 0.25 * (d[a] + d[b]);

        proj_pos += theta_ab * (Gab + Gba);
      }
    return proj_pos;
  }
  else
    mooseError("positiveProjectionEigenDecomposition is only available for ordered tensor "
               "component types");
}

template <typename T>
T
RankTwoTensorTempl<T>::sin3Lode(const T & r0, const T & r0_value) const
{
  if constexpr (MooseUtils::IsLikeReal<T>::value)
  {
    T bar = secondInvariant();
    if (bar <= r0)
      // in this case the Lode angle is not defined
      return r0_value;
    else
      // the min and max here gaurd against precision-loss when bar is tiny but nonzero.
      return std::max(std::min(-1.5 * std::sqrt(3.0) * thirdInvariant() / std::pow(bar, 1.5), 1.0),
                      -1.0);
  }
  else
    mooseError("sin3Lode is only available for ordered tensor component types");
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::dsin3Lode(const T & r0) const
{
  if constexpr (MooseUtils::IsLikeReal<T>::value)
  {
    T bar = secondInvariant();
    if (bar <= r0)
      return RankTwoTensorTempl<T>();
    else
      return -1.5 * std::sqrt(3.0) *
             (dthirdInvariant() / std::pow(bar, 1.5) -
              1.5 * dsecondInvariant() * thirdInvariant() / std::pow(bar, 2.5));
  }
  else
    mooseError("dsin3Lode is only available for ordered tensor component types");
}

template <typename T>
RankFourTensorTempl<T>
RankTwoTensorTempl<T>::d2sin3Lode(const T & r0) const
{
  if constexpr (MooseUtils::IsLikeReal<T>::value)
  {
    T bar = secondInvariant();
    if (bar <= r0)
      return RankFourTensorTempl<T>();

    T J3 = thirdInvariant();
    RankTwoTensorTempl<T> dII = dsecondInvariant();
    RankTwoTensorTempl<T> dIII = dthirdInvariant();
    RankFourTensorTempl<T> deriv = d2thirdInvariant() / std::pow(bar, 1.5) -
                                   1.5 * d2secondInvariant() * J3 / std::pow(bar, 2.5);

    for (unsigned i = 0; i < N; ++i)
      for (unsigned j = 0; j < N; ++j)
        for (unsigned k = 0; k < N; ++k)
          for (unsigned l = 0; l < N; ++l)
            deriv(i, j, k, l) += (-1.5 * dII(i, j) * dIII(k, l) - 1.5 * dIII(i, j) * dII(k, l)) /
                                     std::pow(bar, 2.5) +
                                 1.5 * 2.5 * dII(i, j) * dII(k, l) * J3 / std::pow(bar, 3.5);

    deriv *= -1.5 * std::sqrt(3.0);
    return deriv;
  }
  else
    mooseError("d2sin3Lode is only available for ordered tensor component types");
}

template <typename T>
template <int n, int o, int p, int q>
RankFourTensorTempl<T>
RankTwoTensorTempl<T>::times(const RankTwoTensorTempl<T> & b) const
{
  RankFourTensorTempl<T> result;
  std::size_t x[4];
  for (x[0] = 0; x[0] < N; ++x[0])
    for (x[1] = 0; x[1] < N; ++x[1])
      for (x[2] = 0; x[2] < N; ++x[2])
        for (x[3] = 0; x[3] < N; ++x[3])
          result(x[0], x[1], x[2], x[3]) = (*this)(x[n], x[o]) * b(x[p], x[q]);

  return result;
}

template <typename T>
template <int n, int o, int p, int q, int r, int s>
RankFourTensorTempl<T>
RankTwoTensorTempl<T>::times(const RankFourTensorTempl<T> & b) const
{
  RankFourTensorTempl<T> result;
  std::size_t x[5];
  for (x[0] = 0; x[0] < N; ++x[0])
    for (x[1] = 0; x[1] < N; ++x[1])
      for (x[2] = 0; x[2] < N; ++x[2])
        for (x[3] = 0; x[3] < N; ++x[3])
          for (x[4] = 0; x[4] < N; ++x[4])
            result(x[0], x[1], x[2], x[3]) += (*this)(x[n], x[o]) * b(x[p], x[q], x[r], x[s]);

  return result;
}
