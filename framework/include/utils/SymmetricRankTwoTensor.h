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
#include "ADRankTwoTensorForward.h"
#include "ADSymmetricRankTwoTensorForward.h"
#include "ADSymmetricRankFourTensorForward.h"
#include "MooseUtils.h"
#include "MathUtils.h"

#include "libmesh/libmesh.h"
#include "libmesh/tensor_value.h"

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
class VectorValue;
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
void mooseSetToZero<SymmetricRankTwoTensor>(SymmetricRankTwoTensor & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<ADSymmetricRankTwoTensor>(ADSymmetricRankTwoTensor & v);
}

/**
 * SymmetricRankTwoTensorTempl is designed to handle the Stress or Strain Tensor for
 * an anisotropic material. It is designed to reduce the redundancies of the
 * Complete tensor classes for regular mechanics problems and to enable use of the
 * Mandel notation.
 */
template <typename T>
class SymmetricRankTwoTensorTempl
{
public:
  /// For generic programming
  typedef T value_type;

  ///@{ tensor dimension and Mandel vector length
  static constexpr unsigned int Ndim = Moose::dim;
  static constexpr unsigned int N = Ndim + (Ndim * (Ndim - 1)) / 2;
  ///@}

  // Full tensor indices in the Mandel/Voigt representation
  static constexpr unsigned int full_index[6][2] = {{0, 0}, {1, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}};

  // Reverse tensor indices in the Mandel/Voigt representation
  static constexpr unsigned int reverse_index[3][3] = {{0, 5, 4}, {5, 1, 3}, {4, 3, 2}};

  /// returns the 1 or sqrt(2) prefactor in the Mandel notation for the index i ranging from 0-5.
  static constexpr Real mandelFactor(unsigned int i) { return i < Ndim ? 1.0 : MathUtils::sqrt2; }

  // Select initialization
  enum InitMethod
  {
    initNone,
    initIdentity
  };

  /// Default constructor; fills to zero
  SymmetricRankTwoTensorTempl();

  /// Select specific initialization pattern
  SymmetricRankTwoTensorTempl(const InitMethod);

  /**
   * To fill up the 6 entries in the 2nd-order tensor, fillFromInputVector
   * is called with one of the following fill_methods.
   * See the fill*FromInputVector functions for more details
   */
  enum FillMethod
  {
    autodetect = 0,
    isotropic1 = 1,
    diagonal3 = 3,
    symmetric6 = 6
  };

  /// Constructor that proxies the fillFromInputVector method
  SymmetricRankTwoTensorTempl(const std::vector<T> & input) { this->fillFromInputVector(input); };

  /// Initialization list replacement constructors, 6 arguments
  SymmetricRankTwoTensorTempl(
      const T & S11, const T & S22, const T & S33, const T & S23, const T & S13, const T & S12);

  // explicit cast to a full tensor
  explicit operator RankTwoTensorTempl<T>();

private:
  /// Initialization list replacement constructors, 9 arguments (for internal use only)
  SymmetricRankTwoTensorTempl(const T & S11,
                              const T & S21,
                              const T & S31,
                              const T & S12,
                              const T & S22,
                              const T & S32,
                              const T & S13,
                              const T & S23,
                              const T & S33);

  // internal use named constructor that does not apply the sqrt(2) factors
  static SymmetricRankTwoTensorTempl fromRawComponents(
      const T & S11, const T & S22, const T & S33, const T & S23, const T & S13, const T & S12);

public:
  /// Copy assignment operator must be defined if used
  SymmetricRankTwoTensorTempl(const SymmetricRankTwoTensorTempl<T> & a) = default;

  /// Copy constructor from TensorValue<T>
  explicit SymmetricRankTwoTensorTempl(const TensorValue<T> & a);

  /// Copy constructor from TypeTensor<T>
  explicit SymmetricRankTwoTensorTempl(const TypeTensor<T> & a);

  /// Construct from other template
  template <typename T2>
  SymmetricRankTwoTensorTempl(const SymmetricRankTwoTensorTempl<T2> & a)
  {
    for (const auto i : make_range(N))
      _vals[i] = a(i);
  }

  // Named constructors
  static SymmetricRankTwoTensorTempl identity()
  {
    return SymmetricRankTwoTensorTempl(initIdentity);
  }

  /// named constructor for initializing symmetrically
  static SymmetricRankTwoTensorTempl
  initializeSymmetric(const TypeVector<T> & v0, const TypeVector<T> & v1, const TypeVector<T> & v2);

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

  /// fillFromInputVector takes 1, 3, or 6 inputs to fill in the symmmetric Rank-2 tensor.
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method = autodetect);

  /**
   * fillFromScalarVariable takes FIRST/THIRD/SIXTH order scalar variable to fill in the Rank-2
   * tensor.
   */
  void fillFromScalarVariable(const VariableValue & scalar_variable);

  /// Gets the raw value for the index specified.  Takes index = 0,1,2,3,4,5
  inline T & operator()(unsigned int i) { return _vals[i]; }

  /**
   * Gets the raw value for the index specified.  Takes index = 0,1,2
   * used for const
   */
  inline T operator()(unsigned int i) const { return _vals[i]; }

  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   */
  inline T operator()(unsigned int i, unsigned int j) const
  {
    const auto a = reverse_index[i][j];
    return _vals[a] / mandelFactor(a);
  }

  /// get the specified row of the tensor
  VectorValue<T> row(const unsigned int n) const;

  /// get the specified column of the tensor
  VectorValue<T> column(const unsigned int n) const { return row(n); }

  /// return the matrix multiplied with its transpose A*A^T (guaranteed symmetric)
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T> timesTranspose(const RankTwoTensorTempl<T> &);
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T>
  timesTranspose(const SymmetricRankTwoTensorTempl<T> &);

  /// return the matrix multiplied with its transpose A^T*A (guaranteed symmetric)
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T> transposeTimes(const RankTwoTensorTempl<T> &);
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T>
  transposeTimes(const SymmetricRankTwoTensorTempl<T> &);

  /// return the matrix plus its transpose A-A^T (guaranteed symmetric)
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T> plusTranspose(const RankTwoTensorTempl<T> &);
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T>
  plusTranspose(const SymmetricRankTwoTensorTempl<T> &);

  /// Returns the matrix squared
  SymmetricRankTwoTensorTempl<T> square() const;

  /// Returns the trace
  T tr() const { return _vals[0] + _vals[1] + _vals[2]; }

  /// Set all components to zero
  void zero();

  /**
   * rotates the tensor data given a rank two tensor rotation tensor
   * _vals[i][j] = R_ij * R_jl * _vals[k][l]
   * @param R rotation matrix as a TypeTensor
   */
  void rotate(const TypeTensor<T> & R);

  /**
   * Returns a matrix that is the transpose of the matrix this
   * was called on. This is a non-operation.
   */
  SymmetricRankTwoTensorTempl<T> transpose() const;

  /// sets _vals to a, and returns _vals
  template <typename T2>
  SymmetricRankTwoTensorTempl<T> & operator=(const SymmetricRankTwoTensorTempl<T2> & a);

  /**
   * Assignment-from-scalar operator.  Used only to zero out vectors.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<ScalarTraits<Scalar>::value, SymmetricRankTwoTensorTempl &>::type
  operator=(const Scalar & libmesh_dbg_var(p))
  {
    libmesh_assert_equal_to(p, Scalar(0));
    this->zero();
    return *this;
  }

  /// adds a to _vals
  SymmetricRankTwoTensorTempl<T> & operator+=(const SymmetricRankTwoTensorTempl<T> & a);

  /// returns _vals + a
  template <typename T2>
  SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator+(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// sets _vals -= a and returns vals
  SymmetricRankTwoTensorTempl<T> & operator-=(const SymmetricRankTwoTensorTempl<T> & a);

  /// returns _vals - a
  template <typename T2>
  SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator-(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// returns -_vals
  SymmetricRankTwoTensorTempl<T> operator-() const;

  /// performs _vals *= a
  SymmetricRankTwoTensorTempl<T> & operator*=(const T & a);

  /// returns _vals*a
  template <typename T2>
  auto operator*(const T2 & a) const ->
      typename std::enable_if<ScalarTraits<T2>::value,
                              SymmetricRankTwoTensorTempl<decltype(T() * T2())>>::type;

  /// performs _vals /= a
  SymmetricRankTwoTensorTempl<T> & operator/=(const T & a);

  /// returns _vals/a
  template <typename T2>
  auto operator/(const T2 & a) const ->
      typename std::enable_if<ScalarTraits<T2>::value,
                              SymmetricRankTwoTensorTempl<decltype(T() / T2())>>::type;

  /// Defines multiplication with a vector to get a vector
  template <typename T2>
  TypeVector<typename CompareTypes<T, T2>::supertype> operator*(const TypeVector<T2> & a) const;

  /// Defines logical equality with another SymmetricRankTwoTensorTempl<T2>
  template <typename T2>
  bool operator==(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// Test for symmetry. Surprisingly this is always true.
  bool isSymmetric() const { return true; }

  /// Defines logical inequality with another SymmetricRankTwoTensorTempl<T2>
  template <typename T2>
  bool operator!=(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// Sets _vals to the values in a ColumnMajorMatrix (must be 3x3)
  SymmetricRankTwoTensorTempl<T> & operator=(const ColumnMajorMatrixTempl<T> & a);

  /// returns _vals_ij * a_ij (sum on i, j)
  T doubleContraction(const SymmetricRankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_ij * b_kl
  SymmetricRankFourTensorTempl<T> outerProduct(const SymmetricRankTwoTensorTempl<T> & a) const;

  /// return positive projection tensor of eigen-decomposition
  SymmetricRankFourTensorTempl<T>
  positiveProjectionEigenDecomposition(std::vector<T> &, RankTwoTensorTempl<T> &) const;

  /// returns A_ij - de_ij*tr(A)/3, where A are the _vals
  SymmetricRankTwoTensorTempl<T> deviatoric() const;

  /// returns the trace of the tensor, ie _vals[i][i] (sum i = 0, 1, 2)
  T trace() const;

  /// retuns the inverse of the tensor
  SymmetricRankTwoTensorTempl<T> inverse() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d(trace)/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> dtrace() const;

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
   * Denote the _vals[i][j] by A_ij, then this returns
   * d(secondInvariant)/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> dsecondInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d^2(secondInvariant)/dA_ij/dA_kl
   */
  SymmetricRankFourTensorTempl<T> d2secondInvariant() const;

  /**
   * Sin(3*Lode_angle)
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
  SymmetricRankTwoTensorTempl<T> dsin3Lode(const T & r0) const;

  /**
   * Denote the _vals[i][j] by A_ij, then
   * S_ij = A_ij - de_ij*tr(A)/3
   * Then this returns det(S + S.transpose())/2
   * Note the explicit symmeterisation
   */
  T thirdInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then
   * this returns d(thirdInvariant()/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> dthirdInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d^2(thirdInvariant)/dA_ij/dA_kl
   */
  SymmetricRankFourTensorTempl<T> d2thirdInvariant() const;

  // determinant of the tensor
  T det() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d(det)/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> ddet() const;

  /// Print the rank two tensor
  void print(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the DualReal rank two tensor
  void printReal(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the DualReal rank two tensor along with its first nDual dual numbers
  void printDualReal(unsigned int nDual, std::ostream & stm = Moose::out) const;

  /// Add identity times a to _vals
  void addIa(const T & a);

  /// Sqrt(_vals[i][j]*_vals[i][j])
  T L2norm() const;

  /**
   * sets _vals[0][0], _vals[0][1], _vals[1][0], _vals[1][1] to input,
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
   * This function initializes random seed based on a user-defined number.
   */
  static void initRandom(unsigned int);

  /**
   * This function generates a random symmetric rank two tensor.
   * The first real scales the random number.
   * The second real offsets the uniform random number
   */
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T> genRandomSymmTensor(T, T);

  /// SymmetricRankTwoTensorTempl<T> from outer product of a vector with itself
  [[nodiscard]] static SymmetricRankTwoTensorTempl<T> selfOuterProduct(const TypeVector<T> &);

  /// returns this_ij * b_ijkl
  SymmetricRankTwoTensorTempl<T>
  initialContraction(const SymmetricRankFourTensorTempl<T> & b) const;

  /// set the tensor to the identity matrix
  void setToIdentity();

protected:
  /**
   * Uses the petscblaslapack.h LAPACKsyev_ routine to find, for symmetric _vals:
   *  (1) the eigenvalues (if calculation_type == "N")
   *  (2) the eigenvalues and eigenvectors (if calculation_type == "V")
   * @param calculation_type If "N" then calculation eigenvalues only
   * @param eigvals Eigenvalues are placed in this array, in ascending order
   * @param a Eigenvectors are placed in this array if calculation_type == "V".
   * See code in dsymmetricEigenvalues for extracting eigenvectors from the a output.
   */
  void syev(const char * calculation_type, std::vector<T> & eigvals, std::vector<T> & a) const;

private:
  static constexpr std::array<Real, N> identityCoords = {{1, 1, 1, 0, 0, 0}};

  // tensor components
  std::array<T, N> _vals;

  template <class T2>
  friend void dataStore(std::ostream &, SymmetricRankTwoTensorTempl<T2> &, void *);

  template <class T2>
  friend void dataLoad(std::istream &, SymmetricRankTwoTensorTempl<T2> &, void *);
  template <class T2>
  friend class SymmetricRankFourTensorTempl;
};

namespace MetaPhysicL
{
template <typename T>
struct RawType<SymmetricRankTwoTensorTempl<T>>
{
  typedef SymmetricRankTwoTensorTempl<typename RawType<T>::value_type> value_type;

  static value_type value(const SymmetricRankTwoTensorTempl<T> & in)
  {
    value_type ret;
    for (unsigned int i = 0; i < SymmetricRankTwoTensorTempl<T>::N; ++i)
      ret(i) = raw_value(in(i));

    return ret;
  }
};
}

template <typename T>
template <typename T2>
auto
SymmetricRankTwoTensorTempl<T>::operator*(const T2 & a) const ->
    typename std::enable_if<ScalarTraits<T2>::value,
                            SymmetricRankTwoTensorTempl<decltype(T() * T2())>>::type
{
  SymmetricRankTwoTensorTempl<decltype(T() * T2())> result;
  for (const auto i : make_range(N))
    result._vals[i] = _vals[i] * a;
  return result;
}

template <typename T>
template <typename T2>
auto
SymmetricRankTwoTensorTempl<T>::operator/(const T2 & a) const ->
    typename std::enable_if<ScalarTraits<T2>::value,
                            SymmetricRankTwoTensorTempl<decltype(T() / T2())>>::type
{
  SymmetricRankTwoTensorTempl<decltype(T() / T2())> result;
  for (const auto i : make_range(N))
    result._vals[i] = _vals[i] / a;
  return result;
}

/// Defines multiplication with a vector to get a vector
template <typename T>
template <typename T2>
TypeVector<typename CompareTypes<T, T2>::supertype>
SymmetricRankTwoTensorTempl<T>::operator*(const TypeVector<T2> & a) const
{
  TypeVector<typename CompareTypes<T, T2>::supertype> ret;
  ret(0) = a(0) * _vals[0] + a(1) * _vals[5] + a(2) * _vals[4];
  ret(1) = a(0) * _vals[5] + a(1) * _vals[1] + a(2) * _vals[3];
  ret(2) = a(0) * _vals[4] + a(1) * _vals[3] + a(2) * _vals[2];
}

template <typename T>
template <typename T2>
bool
SymmetricRankTwoTensorTempl<T>::operator==(const SymmetricRankTwoTensorTempl<T2> & a) const
{
  for (std::size_t i = 0; i < N; ++i)
    if (_vals[i] != a._vals[i])
      return false;
  return true;
}

template <typename T>
template <typename T2>
bool
SymmetricRankTwoTensorTempl<T>::operator!=(const SymmetricRankTwoTensorTempl<T2> & a) const
{
  for (std::size_t i = 0; i < N; ++i)
    if (_vals[i] != a._vals[i])
      return true;
  return false;
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::positiveProjectionEigenDecomposition(
    std::vector<T> & eigval, RankTwoTensorTempl<T> & eigvec) const
{
  // The calculate of projection tensor follows
  // C. Miehe and M. Lambrecht, Commun. Numer. Meth. Engng 2001; 17:337~353

  // Compute eigenvectors and eigenvalues of this tensor
  if (MooseUtils::IsLikeReal<T>::value)
  {
    this->symmetricEigenvaluesEigenvectors(eigval, eigvec);

    // Separate out positive and negative eigen values
    std::array<T, N> epos;
    std::array<T, N> d;
    for (unsigned int i = 0; i < N; ++i)
    {
      epos[i] = (std::abs(eigval[i]) + eigval[i]) / 2.0;
      d[i] = 0 < eigval[i] ? 1.0 : 0.0;
    }

    // projection tensor
    SymmetricRankFourTensorTempl<T> proj_pos;

    for (unsigned int a = 0; a < Ndim; ++a)
    {
      const auto Ma = SymmetricRankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(a));
      proj_pos += d[a] * Ma.outerProduct(Ma);
    }

    for (const auto a : make_range(Ndim))
      for (const auto b : make_range(a))
      {
        const auto Ma = SymmetricRankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(a));
        const auto Mb = SymmetricRankTwoTensorTempl<T>::selfOuterProduct(eigvec.column(b));

        SymmetricRankFourTensorTempl<T> Gabba(SymmetricRankFourTensorTempl<T>::initNone);
        for (const auto aa : make_range(N))
          for (const auto bb : make_range(N))
          {
            const auto i = SymmetricRankFourTensorTempl<T>::full_index[aa][bb][0];
            const auto j = SymmetricRankFourTensorTempl<T>::full_index[aa][bb][1];
            const auto k = SymmetricRankFourTensorTempl<T>::full_index[aa][bb][2];
            const auto l = SymmetricRankFourTensorTempl<T>::full_index[aa][bb][3];

            Gabba(aa, bb) = (Ma(i, k) * Mb(j, l) + Ma(i, l) * Mb(j, k) + Ma(j, l) * Mb(i, k) +
                             Ma(j, k) * Mb(i, l)) *
                            SymmetricRankFourTensorTempl<T>::mandelFactor(aa, bb);
          }

        T theta_ab;
        if (!MooseUtils::relativeFuzzyEqual(eigval[a], eigval[b]))
          theta_ab = 0.5 * (epos[a] - epos[b]) / (eigval[a] - eigval[b]);
        else
          theta_ab = 0.25 * (d[a] + d[b]);

        proj_pos += theta_ab * Gabba;
      }
    return proj_pos;
  }
  else
    mooseError("positiveProjectionEigenDecomposition is only available for ordered tensor "
               "component types");
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::sin3Lode(const T & r0, const T & r0_value) const
{
  if (MooseUtils::IsLikeReal<T>::value)
  {
    T bar = secondInvariant();
    if (bar <= r0)
      // in this case the Lode angle is not defined
      return r0_value;
    else
      // the min and max here gaurd against precision-loss when bar is tiny but nonzero.
      return std::max(std::min(thirdInvariant() * -1.5 * std::sqrt(3.0) / std::pow(bar, 1.5), 1.0),
                      -1.0);
  }
  else
    mooseError("sin3Lode is only available for ordered tensor component types");
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::dsin3Lode(const T & r0) const
{
  if (MooseUtils::IsLikeReal<T>::value)
  {
    T bar = secondInvariant();
    if (bar <= r0)
      return SymmetricRankTwoTensorTempl<T>();
    else
      return -1.5 * std::sqrt(3.0) *
             (dthirdInvariant() / std::pow(bar, 1.5) -
              1.5 * dsecondInvariant() * thirdInvariant() / std::pow(bar, 2.5));
  }
  else
    mooseError("dsin3Lode is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
SymmetricRankTwoTensorTempl<T> &
SymmetricRankTwoTensorTempl<T>::operator=(const SymmetricRankTwoTensorTempl<T2> & a)
{
  for (const auto i : make_range(N))
    (*this)(i) = a(i);
  return *this;
}

// non-member operators

template <typename T, typename Scalar>
inline typename std::enable_if_t<
    ScalarTraits<Scalar>::value,
    SymmetricRankTwoTensorTempl<typename CompareTypes<T, Scalar>::supertype>>
operator*(const Scalar & factor, const SymmetricRankTwoTensorTempl<T> & t)
{
  return t * factor;
}
