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
class RankTwoTensorTempl : public TensorValue<T>
{
public:
  ///@{ tensor dimension and dimension squared
  static constexpr unsigned int N = Moose::dim;
  static constexpr unsigned int N2 = N * N;
  ///@}

  // Select initialization
  enum InitMethod
  {
    initNone,
    initIdentity
  };

  /// Default constructor; fills to zero
  RankTwoTensorTempl();

  /// Select specific initialization pattern
  RankTwoTensorTempl(const InitMethod);

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

  /// Convenience enum to specify indices in templated products
  usingTensorIndices(i, j, k, l, m);

  // Deprecated constructor (replaced by initializeFromRows)
  RankTwoTensorTempl(const TypeVector<T> & row1,
                     const TypeVector<T> & row2,
                     const TypeVector<T> & row3);

  /**
   * Named constructor for initializing symetrically. The supplied vectors are
   * used as row and column vectors to construct two tensors respectively, that
   * are averaged to create a symmetric tensor.
   */
  [[nodiscard]] static RankTwoTensorTempl
  initializeSymmetric(const TypeVector<T> & v0, const TypeVector<T> & v1, const TypeVector<T> & v2);

  /// Named constructor for initializing from row vectors
  [[nodiscard]] static RankTwoTensorTempl initializeFromRows(const TypeVector<T> & row0,
                                                             const TypeVector<T> & row1,
                                                             const TypeVector<T> & row2);

  /// Named constructor for initializing from column vectors
  [[nodiscard]] static RankTwoTensorTempl initializeFromColumns(const TypeVector<T> & col0,
                                                                const TypeVector<T> & col1,
                                                                const TypeVector<T> & col2);

  /// Constructor that proxies the fillFromInputVector method
  RankTwoTensorTempl(const std::vector<T> & input) { this->fillFromInputVector(input); };

  /// Initialization list replacement constructors, 6 arguments
  RankTwoTensorTempl(
      const T & S11, const T & S22, const T & S33, const T & S23, const T & S13, const T & S12);

  /// Initialization list replacement constructors, 9 arguments
  RankTwoTensorTempl(const T & S11,
                     const T & S21,
                     const T & S31,
                     const T & S12,
                     const T & S22,
                     const T & S32,
                     const T & S13,
                     const T & S23,
                     const T & S33);

  /// Copy assignment operator must be defined if used
  RankTwoTensorTempl(const RankTwoTensorTempl<T> & a) = default;

  /// Copy constructor from TensorValue<T>
  RankTwoTensorTempl(const TensorValue<T> & a) : TensorValue<T>(a) {}

  /// Copy constructor from TypeTensor<T>
  RankTwoTensorTempl(const TypeTensor<T> & a) : TensorValue<T>(a) {}

  /// Copy constructor from SymmetricRankTwoTensor (delegates)
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

  /// Construct from other template
  template <typename T2>
  RankTwoTensorTempl(const RankTwoTensorTempl<T2> & a) : TensorValue<T>(a)
  {
  }

  // Named constructors
  [[nodiscard]] static RankTwoTensorTempl Identity() { return RankTwoTensorTempl(initIdentity); }

  /// Static method for use in validParams for getting the "fill_method"
  [[nodiscard]] static MooseEnum fillMethodEnum();

  /**
   * fillFromInputVector takes 6 or 9 inputs to fill in the Rank-2 tensor.
   * If 6 inputs, then symmetry is assumed S_ij = S_ji, and
   *   _coords[0][0] = input[0]
   *   _coords[1][1] = input[1]
   *   _coords[2][2] = input[2]
   *   _coords[1][2] = input[3]
   *   _coords[0][2] = input[4]
   *   _coords[0][1] = input[5]
   * If 9 inputs then input order is [0][0], [1][0], [2][0], [0][1], [1][1], ..., [2][2]
   */
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method = autodetect);

  /**
   * fillFromScalarVariable takes FIRST/THIRD/SIXTH order scalar variable to fill in the Rank-2
   * tensor.
   */
  void fillFromScalarVariable(const VariableValue & scalar_variable);

  /// returns _coords[i][c], ie, column c, with c = 0, 1, 2
  VectorValue<T> column(const unsigned int c) const;

  /// return the matrix multiplied with its transpose A*A^T (guaranteed symmetric)
  [[nodiscard]] static RankTwoTensorTempl<T> timesTranspose(const RankTwoTensorTempl<T> &);

  /// return the matrix multiplied with its transpose A^T*A (guaranteed symmetric)
  [[nodiscard]] static RankTwoTensorTempl<T> transposeTimes(const RankTwoTensorTempl<T> &);

  /// return the matrix plus its transpose A+A^T (guaranteed symmetric)
  [[nodiscard]] static RankTwoTensorTempl<T> plusTranspose(const RankTwoTensorTempl<T> &);

  /**
   * Returns the matrix squared
   */
  RankTwoTensorTempl<T> square() const;

  /**
   * Returns a rotated version of the tensor data given a rank two tensor rotation tensor
   * _coords[i][j] = R_ij * R_jl * _coords[k][l]
   * @param R rotation matrix as another RankTwoTensorTempl
   */
  RankTwoTensorTempl<T> rotated(const RankTwoTensorTempl<T> & R) const;

  /**
   * rotates the tensor data given a rank two tensor rotation tensor
   * _coords[i][j] = R_ij * R_jl * _coords[k][l]
   * @param R rotation matrix as a RankTwoTensorTempl
   */
  void rotate(const RankTwoTensorTempl<T> & R);

  /**
   * rotates the tensor data anticlockwise around the z-axis
   * @param a angle in radians
   */
  RankTwoTensorTempl<T> rotateXyPlane(T a);

  /**
   * Returns a matrix that is the transpose of the matrix this
   * was called on.
   */
  RankTwoTensorTempl<T> transpose() const;

  /// sets _coords to a, and returns _coords
  RankTwoTensorTempl<T> & operator=(const RankTwoTensorTempl<T> & a);

  /**
   * Assignment-from-scalar operator.  Used only to zero out vectors.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<ScalarTraits<Scalar>::value, RankTwoTensorTempl &>::type
  operator=(const Scalar & libmesh_dbg_var(p))
  {
    libmesh_assert_equal_to(p, Scalar(0));
    this->zero();
    return *this;
  }

  /// adds a to _coords
  RankTwoTensorTempl<T> & operator+=(const RankTwoTensorTempl<T> & a);

  /// returns _coords + a
  template <typename T2>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator+(const TypeTensor<T2> & a) const;

  /// sets _coords -= a and returns vals
  RankTwoTensorTempl<T> & operator-=(const RankTwoTensorTempl<T> & a);

  /// returns _coords - a
  template <typename T2>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator-(const TypeTensor<T2> & a) const;

  /// returns -_coords
  RankTwoTensorTempl<T> operator-() const;

  /// performs _coords *= a
  RankTwoTensorTempl<T> & operator*=(const T & a);

  /// returns _coords*a
  template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type = 0>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype> operator*(const T2 & a) const;

  /// performs _coords /= a
  RankTwoTensorTempl<T> & operator/=(const T & a);

  /// returns _coords/a
  template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type = 0>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype> operator/(const T2 & a) const;

  /// Defines multiplication with a vector to get a vector
  template <typename T2>
  TypeVector<typename CompareTypes<T, T2>::supertype> operator*(const TypeVector<T2> & a) const;

  /// Defines multiplication with a TypeTensor<T>
  template <typename T2>
  RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator*(const TypeTensor<T2> & a) const;

  /// Defines multiplication with a TypeTensor<T>
  RankTwoTensorTempl<T> & operator*=(const TypeTensor<T> & a);

  /// Defines logical equality with another RankTwoTensorTempl<T>
  bool operator==(const RankTwoTensorTempl<T> & a) const;

  /// Sets _coords to the values in a ColumnMajorMatrix (must be 3x3)
  RankTwoTensorTempl<T> & operator=(const ColumnMajorMatrixTempl<T> & a);

  /// returns _coords_ij * a_ij (sum on i, j)
  T doubleContraction(const RankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_no * b_pq
  template <int n, int o, int p, int q>
  RankFourTensorTempl<T> times(const RankTwoTensorTempl<T> & b) const;

  /// returns C_ijkl = a_no * b_pqrs
  template <int n, int o, int p, int q, int r, int s>
  RankFourTensorTempl<T> times(const RankFourTensorTempl<T> & b) const;

  /// returns C_ijkl = a_ij * b_kl
  RankFourTensorTempl<T> outerProduct(const RankTwoTensorTempl<T> & b) const
  {
    return times<i, j, k, l>(b);
  }

  /// returns C_ikl = a_ij * b_jkl (single contraction over index j)
  RankThreeTensorTempl<T> contraction(const RankThreeTensorTempl<T> & b) const;

  /// returns C_ijk = a_jk * b_i
  RankThreeTensorTempl<T> mixedProductJkI(const VectorValue<T> & b) const;

  /// return positive projection tensor of eigen-decomposition
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
  positiveProjectionEigenDecomposition(std::vector<T> &, RankTwoTensorTempl<T> &) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
  positiveProjectionEigenDecomposition(std::vector<T> &, RankTwoTensorTempl<T> &) const;

  /// returns A_ij - de_ij*tr(A)/3, where A are the _coords
  RankTwoTensorTempl<T> deviatoric() const;

  /// returns the trace of the tensor, ie _coords[i][i] (sum i = 0, 1, 2)
  T trace() const;

  /// retuns the inverse of the tensor
  RankTwoTensorTempl<T> inverse() const;

  /**
   * Denote the _coords[i][j] by A_ij, then this returns
   * d(trace)/dA_ij
   */
  RankTwoTensorTempl<T> dtrace() const;

  /**
   * Calculates the second invariant (I2) of a tensor
   */
  T generalSecondInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then
   * S_ij = A_ij - de_ij*tr(A)/3
   * Then this returns (S_ij + S_ji)*(S_ij + S_ji)/8
   * Note the explicit symmeterisation
   */
  T secondInvariant() const;

  /**
   * Denote the _coords[i][j] by A_ij, then this returns
   * d(secondInvariant)/dA_ij
   */
  RankTwoTensorTempl<T> dsecondInvariant() const;

  /**
   * Denote the _coords[i][j] by A_ij, then this returns
   * d^2(secondInvariant)/dA_ij/dA_kl
   */
  RankFourTensorTempl<T> d2secondInvariant() const;

  /**
   * Sin(3*Lode_angle)
   * If secondInvariant() <= r0 then return r0_value
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, T>::type
  sin3Lode(const T & r0, const T & r0_value) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, T>::type
  sin3Lode(const T & r0, const T & r0_value) const;

  /**
   * d(sin3Lode)/dA_ij
   * If secondInvariant() <= r0 then return zero
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, RankTwoTensorTempl<T>>::type
  dsin3Lode(const T & r0) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, RankTwoTensorTempl<T>>::type
  dsin3Lode(const T & r0) const;

  /**
   * d^2(sin3Lode)/dA_ij/dA_kl
   * If secondInvariant() <= r0 then return zero
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
  d2sin3Lode(const T & r0) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
  d2sin3Lode(const T & r0) const;

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

  /// Print the rank two tensor
  void print(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the DualReal rank two tensor
  void printReal(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the DualReal rank two tensor along with its first nDual dual numbers
  void printDualReal(unsigned int nDual, std::ostream & stm = Moose::out) const;

  /// Add identity times a to _coords
  void addIa(const T & a);

  /// Sqrt(_coords[i][j]*_coords[i][j])
  T L2norm() const;

  /**
   * sets _coords[0][0], _coords[0][1], _coords[1][0], _coords[1][1] to input,
   * and the remainder to zero
   */
  void surfaceFillFromInputVector(const std::vector<T> & input);

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

  /**
   * This function initializes random seed based on a user-defined number.
   */
  static void initRandom(unsigned int);

  /**
   * This function generates a random unsymmetric rank two tensor.
   * The first real scales the random number.
   * The second real offsets the uniform random number
   */
  [[nodiscard]] static RankTwoTensorTempl<T> genRandomTensor(T, T);

  /**
   * This function generates a random symmetric rank two tensor.
   * The first real scales the random number.
   * The second real offsets the uniform random number
   */
  [[nodiscard]] static RankTwoTensorTempl<T> genRandomSymmTensor(T, T);

  /// RankTwoTensorTempl<T> from outer product of vectors (sets the current tensor and should be deprecated)
  void vectorOuterProduct(const TypeVector<T> &, const TypeVector<T> &);

  /// RankTwoTensorTempl<T> from outer product of vectors
  [[nodiscard]] static RankTwoTensorTempl<T> outerProduct(const TypeVector<T> &,
                                                          const TypeVector<T> &);

  /// RankTwoTensorTempl<T> from outer product of a vector with itself
  [[nodiscard]] static RankTwoTensorTempl<T> selfOuterProduct(const TypeVector<T> &);

  /// Return real tensor of a rank two tensor
  void fillRealTensor(TensorValue<T> &);

  ///Assigns value to the columns of a specified row
  void fillRow(unsigned int, const TypeVector<T> &);

  ///Assigns value to the rows of a specified column
  void fillColumn(unsigned int, const TypeVector<T> &);

  /// returns this_ij * b_ijkl
  RankTwoTensorTempl<T> initialContraction(const RankFourTensorTempl<T> & b) const;

  /// set the tensor to the identity matrix
  void setToIdentity();

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

  using TensorValue<T>::_coords;

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
RankTwoTensorTempl<T>::operator+(const TypeTensor<T2> & b) const
{
  return TensorValue<T>::operator+(b);
}

template <typename T>
template <typename T2>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator-(const TypeTensor<T2> & b) const
{
  return TensorValue<T>::operator-(b);
}

template <typename T>
template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator*(const T2 & b) const
{
  return TensorValue<T>::operator*(b);
}

template <typename T>
template <typename T2>
TypeVector<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator*(const TypeVector<T2> & b) const
{
  return TensorValue<T>::operator*(b);
}

template <typename T>
template <typename T2>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator*(const TypeTensor<T2> & b) const
{
  return TensorValue<T>::operator*(b);
}

template <typename T>
template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type>
RankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
RankTwoTensorTempl<T>::operator/(const T2 & b) const
{
  return TensorValue<T>::operator/(b);
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
RankTwoTensorTempl<T>::positiveProjectionEigenDecomposition(std::vector<T> & eigval,
                                                            RankTwoTensorTempl<T> & eigvec) const
{
  // The calculate of projection tensor follows
  // C. Miehe and M. Lambrecht, Commun. Numer. Meth. Engng 2001; 17:337~353

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

  for (auto a : make_range(N))
    for (auto b : make_range(a))
    {
      const auto Ma = RankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(a));
      const auto Mb = RankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(b));

      Gab = Ma.times<i, k, j, l>(Mb) + Ma.times<i, k, j, l>(Mb);
      Gba = Mb.times<i, k, j, l>(Ma) + Mb.times<i, k, j, l>(Ma);

      T theta_ab;
      if (!MooseUtils::absoluteFuzzyEqual(eigval[a], eigval[b]))
        theta_ab = 0.5 * (epos[a] - epos[b]) / (eigval[a] - eigval[b]);
      else
        theta_ab = 0.25 * (d[a] + d[b]);

      proj_pos += theta_ab * (Gab + Gba);
    }
  return proj_pos;
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
RankTwoTensorTempl<T>::positiveProjectionEigenDecomposition(std::vector<T> &,
                                                            RankTwoTensorTempl<T> &) const
{
  mooseError(
      "positiveProjectionEigenDecomposition is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, T>::type
RankTwoTensorTempl<T>::sin3Lode(const T & r0, const T & r0_value) const
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

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, T>::type
RankTwoTensorTempl<T>::sin3Lode(const T &, const T &) const
{
  mooseError("sin3Lode is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, RankTwoTensorTempl<T>>::type
RankTwoTensorTempl<T>::dsin3Lode(const T & r0) const
{
  T bar = secondInvariant();
  if (bar <= r0)
    return RankTwoTensorTempl<T>();
  else
    return -1.5 * std::sqrt(3.0) *
           (dthirdInvariant() / std::pow(bar, 1.5) -
            1.5 * dsecondInvariant() * thirdInvariant() / std::pow(bar, 2.5));
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, RankTwoTensorTempl<T>>::type
RankTwoTensorTempl<T>::dsin3Lode(const T &) const
{
  mooseError("dsin3Lode is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
RankTwoTensorTempl<T>::d2sin3Lode(const T & r0) const
{
  T bar = secondInvariant();
  if (bar <= r0)
    return RankFourTensorTempl<T>();

  T J3 = thirdInvariant();
  RankTwoTensorTempl<T> dII = dsecondInvariant();
  RankTwoTensorTempl<T> dIII = dthirdInvariant();
  RankFourTensorTempl<T> deriv =
      d2thirdInvariant() / std::pow(bar, 1.5) - 1.5 * d2secondInvariant() * J3 / std::pow(bar, 2.5);

  for (unsigned i = 0; i < N; ++i)
    for (unsigned j = 0; j < N; ++j)
      for (unsigned k = 0; k < N; ++k)
        for (unsigned l = 0; l < N; ++l)
          deriv(i, j, k, l) +=
              (-1.5 * dII(i, j) * dIII(k, l) - 1.5 * dIII(i, j) * dII(k, l)) / std::pow(bar, 2.5) +
              1.5 * 2.5 * dII(i, j) * dII(k, l) * J3 / std::pow(bar, 3.5);

  deriv *= -1.5 * std::sqrt(3.0);
  return deriv;
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, RankFourTensorTempl<T>>::type
RankTwoTensorTempl<T>::d2sin3Lode(const T &) const
{
  mooseError("d2sin3Lode is only available for ordered tensor component types");
}

template <typename T>
template <int i, int j, int k, int l>
RankFourTensorTempl<T>
RankTwoTensorTempl<T>::times(const RankTwoTensorTempl<T> & b) const
{
  RankFourTensorTempl<T> result;
  std::size_t x[4];
  for (x[0] = 0; x[0] < N; ++x[0])
    for (x[1] = 0; x[1] < N; ++x[1])
      for (x[2] = 0; x[2] < N; ++x[2])
        for (x[3] = 0; x[3] < N; ++x[3])
          result(x[0], x[1], x[2], x[3]) = (*this)(x[i], x[j]) * b(x[k], x[l]);

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
