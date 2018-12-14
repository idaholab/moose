//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANKTWOTENSOR_H
#define RANKTWOTENSOR_H

#include "Moose.h"
#include "ADReal.h"

// Any requisite includes here
#include "libmesh/libmesh.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include <petscsys.h>
#include <vector>

// Forward declarations
template <typename>
class RankTwoTensorTempl;
template <typename>
class RankFourTensorTempl;
class MooseEnum;
template <typename T>
class MooseArray;
typedef MooseArray<Real> VariableValue;
template <typename>
class ColumnMajorMatrixTempl;

template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankTwoTensorTempl<Real>>(RankTwoTensorTempl<Real> & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankTwoTensorTempl<ADReal>>(RankTwoTensorTempl<ADReal> & v);

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

  /**
   * Constructor that takes in 3 vectors and uses them to create rows
   * _coords[0][i] = row1(i), _coords[1][i] = row2(i), _coords[2][i] = row3(i)
   */
  RankTwoTensorTempl(const TypeVector<T> & row1,
                     const TypeVector<T> & row2,
                     const TypeVector<T> & row3);

  /// named constructor for initializing from row vectors
  static RankTwoTensorTempl initializeFromRows(const TypeVector<T> & row0,
                                               const TypeVector<T> & row1,
                                               const TypeVector<T> & row2);

  /// named constructor for initializing from column vectors
  static RankTwoTensorTempl initializeFromColumns(const TypeVector<T> & col0,
                                                  const TypeVector<T> & col1,
                                                  const TypeVector<T> & col2);

  /// Constructor that proxies the fillFromInputVector method
  RankTwoTensorTempl(const std::vector<T> & input) { this->fillFromInputVector(input); };

  /// Initialization list replacement constructors, 6 arguments
  RankTwoTensorTempl(T S11, T S22, T S33, T S23, T S13, T S12);

  /// Initialization list replacement constructors, 9 arguments
  RankTwoTensorTempl(T S11, T S21, T S31, T S12, T S22, T S32, T S13, T S23, T S33);

  /// Copy constructor from TensorValue<T>
  RankTwoTensorTempl(const TensorValue<T> & a) : TensorValue<T>(a) {}

  /// Copy constructor from TypeTensor<T>
  RankTwoTensorTempl(const TypeTensor<T> & a) : TensorValue<T>(a) {}

  /// Construct from other template
  template <typename T2>
  RankTwoTensorTempl(const RankTwoTensorTempl<T2> & a) : TensorValue<T>(a)
  {
  }

  // Named constructors
  static RankTwoTensorTempl Identity() { return RankTwoTensorTempl(initIdentity); }

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

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

public:
  /// returns _coords[i][c], ie, column c, with c = 0, 1, 2
  TypeVector<T> column(const unsigned int c) const;

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
  RankTwoTensorTempl<T> operator+(const RankTwoTensorTempl<T> & a) const;

  /// sets _coords -= a and returns vals
  RankTwoTensorTempl<T> & operator-=(const RankTwoTensorTempl<T> & a);

  /// returns _coords - a
  RankTwoTensorTempl<T> operator-(const RankTwoTensorTempl<T> & a) const;

  /// returns -_coords
  RankTwoTensorTempl<T> operator-() const;

  /// performs _coords *= a
  RankTwoTensorTempl<T> & operator*=(const T & a);

  /// returns _coords*a
  RankTwoTensorTempl<T> operator*(const T & a) const;

  /// performs _coords /= a
  RankTwoTensorTempl<T> & operator/=(const T & a);

  /// returns _coords/a
  RankTwoTensorTempl<T> operator/(const T & a) const;

  /// Defines multiplication with a vector to get a vector
  TypeVector<T> operator*(const TypeVector<T> & a) const;

  /// Defines multiplication with a TypeTensor<T>
  RankTwoTensorTempl<T> operator*(const TypeTensor<T> & a) const;

  /// Defines multiplication with a TypeTensor<T>
  RankTwoTensorTempl<T> & operator*=(const TypeTensor<T> & a);

  /// Defines logical equality with another RankTwoTensorTempl<T>
  bool operator==(const RankTwoTensorTempl<T> & a) const;

  /// Sets _coords to the values in a ColumnMajorMatrix (must be 3x3)
  RankTwoTensorTempl<T> & operator=(const ColumnMajorMatrixTempl<T> & a);

  /// returns _coords_ij * a_ij (sum on i, j)
  T doubleContraction(const RankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_ij * b_kl
  RankFourTensorTempl<T> outerProduct(const RankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_ik * b_jl
  RankFourTensorTempl<T> mixedProductIkJl(const RankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_jk * b_il
  RankFourTensorTempl<T> mixedProductJkIl(const RankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_il * b_jk
  RankFourTensorTempl<T> mixedProductIlJk(const RankTwoTensorTempl<T> & a) const;

  /// return positive projection tensor of eigen-decomposition
  RankFourTensorTempl<T> positiveProjectionEigenDecomposition(std::vector<T> & eigval,
                                                              RankTwoTensorTempl<T> & eigvec) const;

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
   * Denote the _coords[i][j] by A_ij, then
   * S_ij = A_ij - de_ij*tr(A)/3
   * Then this returns (S_ij + S_ji)*(S_ij + S_ji)/8
   * Note the explicit symmeterisation
   */
  T generalSecondInvariant() const;

  /**
   * Calculates the second invariant (I2) of a tensor
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

  /// Print the rank two tensor
  void print(std::ostream & stm = Moose::out) const;

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
   * Uses the petscblaslapack.h LAPACKsyev_ routine to find, for symmetric _coords:
   *  (1) the eigenvalues (if calculation_type == "N")
   *  (2) the eigenvalues and eigenvectors (if calculation_type == "V")
   * @param calculation_type If "N" then calculation eigenvalues only
   * @param eigvals Eigenvalues are placed in this array, in ascending order
   * @param a Eigenvectors are placed in this array if calculation_type == "V".
   * See code in dsymmetricEigenvalues for extracting eigenvectors from the a output.
   */
  void syev(const char * calculation_type, std::vector<T> & eigvals, std::vector<T> & a) const;

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
  static RankTwoTensorTempl<T> genRandomTensor(T, T);

  /**
   * This function generates a random symmetric rank two tensor.
   * The first real scales the random number.
   * The second real offsets the uniform random number
   */
  static RankTwoTensorTempl<T> genRandomSymmTensor(T, T);

  /// RankTwoTensorTempl<T> from outer product of vectors
  void vectorOuterProduct(const TypeVector<T> &, const TypeVector<T> &);

  /// Return real tensor of a rank two tensor
  void fillRealTensor(TensorValue<T> &);

  ///Assigns value to the columns of a specified row
  void fillRow(unsigned int, const TypeVector<T> &);

  ///Assigns value to the rows of a specified column
  void fillColumn(unsigned int, const TypeVector<T> &);

  /// returns this_ij * b_ijkl
  RankTwoTensorTempl<T> initialContraction(const RankFourTensorTempl<T> & b) const;

private:
  static constexpr unsigned int N = LIBMESH_DIM;
  static constexpr unsigned int N2 = N * N;

  template <class T2>
  friend void dataStore(std::ostream &, RankTwoTensorTempl<T2> &, void *);

  template <class T2>
  friend void dataLoad(std::istream &, RankTwoTensorTempl<T2> &, void *);
  template <class T2>
  friend class RankFourTensorTempl;
  friend class RankThreeTensor;
};

typedef RankTwoTensorTempl<Real> RankTwoTensor;
typedef RankTwoTensorTempl<ADReal> ADRankTwoTensor;

#endif // RANKTWOTENSOR_H
