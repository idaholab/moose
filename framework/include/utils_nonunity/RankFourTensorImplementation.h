//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankFourTensor.h"

// MOOSE includes
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "MooseEnum.h"
#include "MooseException.h"
#include "MooseUtils.h"
#include "MatrixTools.h"
#include "PermutationTensor.h"

#include "libmesh/utility.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

// Eigen needs LU
#include "Eigen/LU"

// C++ includes
#include <iomanip>
#include <ostream>

namespace MathUtils
{
template <>
void mooseSetToZero<RankFourTensorTempl<Real>>(RankFourTensorTempl<Real> & v);
template <>
void mooseSetToZero<RankFourTensorTempl<DualReal>>(RankFourTensorTempl<DualReal> & v);
}

template <typename T>
MooseEnum
RankFourTensorTempl<T>::fillMethodEnum()
{
  return MooseEnum("antisymmetric symmetric9 symmetric21 general_isotropic symmetric_isotropic "
                   "symmetric_isotropic_E_nu antisymmetric_isotropic axisymmetric_rz general "
                   "principal orthotropic");
}

template <typename T>
RankFourTensorTempl<T>::RankFourTensorTempl()
{
  mooseAssert(N == 3, "RankFourTensorTempl<T> is currently only tested for 3 dimensions.");

  for (auto i : make_range(N4))
    _vals[i] = 0.0;
}

template <typename T>
RankFourTensorTempl<T>::RankFourTensorTempl(const InitMethod init)
{
  unsigned int index = 0;
  switch (init)
  {
    case initNone:
      break;

    case initIdentity:
      zero();
      for (auto i : make_range(N))
        (*this)(i, i, i, i) = 1.0;
      break;

    case initIdentityFour:
      for (auto i : make_range(N))
        for (auto j : make_range(N))
          for (auto k : make_range(N))
            for (auto l : make_range(N))
              _vals[index++] = Real(i == k && j == l);
      break;

    case initIdentitySymmetricFour:
      for (auto i : make_range(N))
        for (auto j : make_range(N))
          for (auto k : make_range(N))
            for (auto l : make_range(N))
              _vals[index++] = 0.5 * Real(i == k && j == l) + 0.5 * Real(i == l && j == k);
      break;

    case initIdentityDeviatoric:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
            {
              _vals[index] = Real(i == k && j == l);
              if ((i == j) && (k == l))
                _vals[index] -= 1.0 / 3.0;
              index++;
            }
      break;

    default:
      mooseError("Unknown RankFourTensorTempl<T> initialization pattern.");
  }
}

template <typename T>
RankFourTensorTempl<T>::RankFourTensorTempl(const std::vector<T> & input, FillMethod fill_method)
{
  fillFromInputVector(input, fill_method);
}

template <typename T>
void
RankFourTensorTempl<T>::zero()
{
  for (auto i : make_range(N4))
    _vals[i] = 0.0;
}

template <typename T>
RankFourTensorTempl<T> &
RankFourTensorTempl<T>::operator=(const RankFourTensorTempl<T> & a)
{
  for (auto i : make_range(N4))
    _vals[i] = a._vals[i];
  return *this;
}

template <typename T>
template <template <typename> class Tensor, typename T2>
auto
RankFourTensorTempl<T>::operator*(const Tensor<T2> & b) const ->
    typename std::enable_if<TwoTensorMultTraits<Tensor, T2>::value,
                            RankTwoTensorTempl<decltype(T() * T2())>>::type
{
  typedef decltype(T() * T2()) ValueType;
  RankTwoTensorTempl<ValueType> result;

  unsigned int index = 0;
  for (unsigned int ij = 0; ij < N2; ++ij)
  {
    ValueType tmp = 0;
    for (unsigned int kl = 0; kl < N2; ++kl)
      tmp += _vals[index++] * b(kl / LIBMESH_DIM, kl % LIBMESH_DIM);
    result._coords[ij] = tmp;
  }

  return result;
}

template <typename T>
RankFourTensorTempl<T> &
RankFourTensorTempl<T>::operator*=(const T & a)
{
  for (auto i : make_range(N4))
    _vals[i] *= a;
  return *this;
}

template <typename T>
RankFourTensorTempl<T> &
RankFourTensorTempl<T>::operator/=(const T & a)
{
  for (auto i : make_range(N4))
    _vals[i] /= a;
  return *this;
}

template <typename T>
RankFourTensorTempl<T> &
RankFourTensorTempl<T>::operator+=(const RankFourTensorTempl<T> & a)
{
  for (auto i : make_range(N4))
    _vals[i] += a._vals[i];
  return *this;
}

template <typename T>
template <typename T2>
auto
RankFourTensorTempl<T>::operator+(const RankFourTensorTempl<T2> & b) const
    -> RankFourTensorTempl<decltype(T() + T2())>
{
  RankFourTensorTempl<decltype(T() + T2())> result;
  for (auto i : make_range(N4))
    result._vals[i] = _vals[i] + b._vals[i];
  return result;
}

template <typename T>
RankFourTensorTempl<T> &
RankFourTensorTempl<T>::operator-=(const RankFourTensorTempl<T> & a)
{
  for (auto i : make_range(N4))
    _vals[i] -= a._vals[i];
  return *this;
}

template <typename T>
template <typename T2>
auto
RankFourTensorTempl<T>::operator-(const RankFourTensorTempl<T2> & b) const
    -> RankFourTensorTempl<decltype(T() - T2())>
{
  RankFourTensorTempl<decltype(T() - T2())> result;
  for (auto i : make_range(N4))
    result._vals[i] = _vals[i] - b._vals[i];
  return result;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::operator-() const
{
  RankFourTensorTempl<T> result;
  for (auto i : make_range(N4))
    result._vals[i] = -_vals[i];
  return result;
}

template <typename T>
template <typename T2>
auto
RankFourTensorTempl<T>::operator*(const RankFourTensorTempl<T2> & b) const
    -> RankFourTensorTempl<decltype(T() * T2())>
{
  typedef decltype(T() * T2()) ValueType;
  RankFourTensorTempl<ValueType> result;

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
          for (auto p : make_range(N))
            for (auto q : make_range(N))
              result(i, j, k, l) += (*this)(i, j, p, q) * b(p, q, k, l);

  return result;
}

template <typename T>
T
RankFourTensorTempl<T>::L2norm() const
{
  T l2 = 0;

  for (auto i : make_range(N4))
    l2 += Utility::pow<2>(_vals[i]);

  return std::sqrt(l2);
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::invSymm() const
{
  mooseError("The invSymm operation calls to LAPACK and only supports plain Real type tensors.");
}

template <>
RankFourTensorTempl<Real>
RankFourTensorTempl<Real>::invSymm() const
{
  unsigned int ntens = N * (N + 1) / 2;
  int nskip = N - 1;

  RankFourTensorTempl<Real> result;
  std::vector<PetscScalar> mat;
  mat.assign(ntens * ntens, 0);

  // We use the LAPACK matrix inversion routine here.  Form the matrix
  //
  // mat[0]  mat[1]  mat[2]  mat[3]  mat[4]  mat[5]
  // mat[6]  mat[7]  mat[8]  mat[9]  mat[10] mat[11]
  // mat[12] mat[13] mat[14] mat[15] mat[16] mat[17]
  // mat[18] mat[19] mat[20] mat[21] mat[22] mat[23]
  // mat[24] mat[25] mat[26] mat[27] mat[28] mat[29]
  // mat[30] mat[31] mat[32] mat[33] mat[34] mat[35]
  //
  // This is filled from the indpendent components of C assuming
  // the symmetry C_ijkl = C_ijlk = C_jikl.
  //
  // If there are two rank-four tensors X and Y then the reason for
  // this filling becomes apparent if we want to calculate
  // X_ijkl*Y_klmn = Z_ijmn
  // For denote the "mat" versions of X, Y and Z by x, y and z.
  // Then
  // z_ab = x_ac*y_cb
  // Eg
  // z_00 = Z_0000 = X_0000*Y_0000 + X_0011*Y_1111 + X_0022*Y_2200 + 2*X_0001*Y_0100 +
  // 2*X_0002*Y_0200 + 2*X_0012*Y_1200   (the factors of 2 come from the assumed symmetries)
  // z_03 = 2*Z_0001 = X_0000*2*Y_0001 + X_0011*2*Y_1101 + X_0022*2*Y_2201 + 2*X_0001*2*Y_0101 +
  // 2*X_0002*2*Y_0201 + 2*X_0012*2*Y_1201
  // z_22 = 2*Z_0102 = X_0100*2*Y_0002 + X_0111*2*X_1102 + X_0122*2*Y_2202 + 2*X_0101*2*Y_0102 +
  // 2*X_0102*2*Y_0202 + 2*X_0112*2*Y_1202
  // Finally, we use LAPACK to find x^-1, and put it back into rank-4 tensor form
  //
  // mat[0] = C(0,0,0,0)
  // mat[1] = C(0,0,1,1)
  // mat[2] = C(0,0,2,2)
  // mat[3] = C(0,0,0,1)*2
  // mat[4] = C(0,0,0,2)*2
  // mat[5] = C(0,0,1,2)*2

  // mat[6] = C(1,1,0,0)
  // mat[7] = C(1,1,1,1)
  // mat[8] = C(1,1,2,2)
  // mat[9] = C(1,1,0,1)*2
  // mat[10] = C(1,1,0,2)*2
  // mat[11] = C(1,1,1,2)*2

  // mat[12] = C(2,2,0,0)
  // mat[13] = C(2,2,1,1)
  // mat[14] = C(2,2,2,2)
  // mat[15] = C(2,2,0,1)*2
  // mat[16] = C(2,2,0,2)*2
  // mat[17] = C(2,2,1,2)*2

  // mat[18] = C(0,1,0,0)
  // mat[19] = C(0,1,1,1)
  // mat[20] = C(0,1,2,2)
  // mat[21] = C(0,1,0,1)*2
  // mat[22] = C(0,1,0,2)*2
  // mat[23] = C(0,1,1,2)*2

  // mat[24] = C(0,2,0,0)
  // mat[25] = C(0,2,1,1)
  // mat[26] = C(0,2,2,2)
  // mat[27] = C(0,2,0,1)*2
  // mat[28] = C(0,2,0,2)*2
  // mat[29] = C(0,2,1,2)*2

  // mat[30] = C(1,2,0,0)
  // mat[31] = C(1,2,1,1)
  // mat[32] = C(1,2,2,2)
  // mat[33] = C(1,2,0,1)*2
  // mat[34] = C(1,2,0,2)*2
  // mat[35] = C(1,2,1,2)*2

  unsigned int index = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          if (i == j)
            mat[k == l ? i * ntens + k : i * ntens + k + nskip + l] += _vals[index];
          else
            // i!=j
            mat[k == l ? (nskip + i + j) * ntens + k : (nskip + i + j) * ntens + k + nskip + l] +=
                _vals[index]; // note the +=, which results in double-counting and is rectified
                              // below
          index++;
        }

  for (unsigned int i = 3; i < ntens; ++i)
    for (auto j : make_range(ntens))
      mat[i * ntens + j] /= 2.0; // because of double-counting above

  // use LAPACK to find the inverse
  MatrixTools::inverse(mat, ntens);

  // build the resulting rank-four tensor
  // using the inverse of the above algorithm
  index = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          if (i == j)
            result._vals[index] =
                k == l ? mat[i * ntens + k] : mat[i * ntens + k + nskip + l] / 2.0;
          else
            // i!=j
            result._vals[index] = k == l ? mat[(nskip + i + j) * ntens + k]
                                         : mat[(nskip + i + j) * ntens + k + nskip + l] / 2.0;
          index++;
        }

  return result;
}

template <typename T>
void
RankFourTensorTempl<T>::rotate(const TypeTensor<T> & R)
{
  RankFourTensorTempl<T> old = *this;

  unsigned int index = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          unsigned int index2 = 0;
          T sum = 0.0;
          for (auto m : make_range(N))
          {
            const T & a = R(i, m);
            for (auto n : make_range(N))
            {
              const T & ab = a * R(j, n);
              for (auto o : make_range(N))
              {
                const T & abc = ab * R(k, o);
                for (auto p : make_range(N))
                  sum += abc * R(l, p) * old._vals[index2++];
              }
            }
          }
          _vals[index++] = sum;
        }
}

template <typename T>
void
RankFourTensorTempl<T>::print(std::ostream & stm) const
{
  for (auto i : make_range(N))
    for (auto j : make_range(N))
    {
      stm << "i = " << i << " j = " << j << '\n';
      for (auto k : make_range(N))
      {
        for (auto l : make_range(N))
          stm << std::setw(15) << (*this)(i, j, k, l) << " ";

        stm << '\n';
      }
    }

  stm << std::flush;
}

template <typename T>
void
RankFourTensorTempl<T>::printReal(std::ostream & stm) const
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      stm << "i = " << i << " j = " << j << '\n';
      for (unsigned int k = 0; k < N; ++k)
      {
        for (unsigned int l = 0; l < N; ++l)
          stm << std::setw(15) << MetaPhysicL::raw_value((*this)(i, j, k, l)) << " ";

        stm << '\n';
      }
    }

  stm << std::flush;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::transposeMajor() const
{
  RankFourTensorTempl<T> result;

  unsigned int index = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
          result._vals[index++] = _vals[k * N3 + i * N + j + l * N2];

  return result;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::transposeIj() const
{
  RankFourTensorTempl<T> result;

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
          result(i, j, k, l) = (*this)(j, i, k, l);

  return result;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::transposeKl() const
{
  RankFourTensorTempl<T> result;

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
          result(i, j, k, l) = (*this)(i, j, l, k);

  return result;
}

template <typename T>
void
RankFourTensorTempl<T>::surfaceFillFromInputVector(const std::vector<T> & input)
{
  zero();

  if (input.size() == 9)
  {
    // then fill from vector C_1111, C_1112, C_1122, C_1212, C_1222, C_1211, C_2211, C_2212, C_2222
    (*this)(0, 0, 0, 0) = input[0];
    (*this)(0, 0, 0, 1) = input[1];
    (*this)(0, 0, 1, 1) = input[2];
    (*this)(0, 1, 0, 1) = input[3];
    (*this)(0, 1, 1, 1) = input[4];
    (*this)(0, 1, 0, 0) = input[5];
    (*this)(1, 1, 0, 0) = input[6];
    (*this)(1, 1, 0, 1) = input[7];
    (*this)(1, 1, 1, 1) = input[8];

    // fill in remainders from C_ijkl = C_ijlk = C_jikl
    (*this)(0, 0, 1, 0) = (*this)(0, 0, 0, 1);
    (*this)(0, 1, 1, 0) = (*this)(0, 1, 0, 1);
    (*this)(1, 0, 0, 0) = (*this)(0, 1, 0, 0);
    (*this)(1, 0, 0, 1) = (*this)(0, 1, 0, 1);
    (*this)(1, 0, 1, 1) = (*this)(0, 1, 1, 1);
    (*this)(1, 0, 0, 0) = (*this)(0, 1, 0, 0);
    (*this)(1, 1, 1, 0) = (*this)(1, 1, 0, 1);
  }
  else if (input.size() == 2)
  {
    // only two independent constants, C_1111 and C_1122
    (*this)(0, 0, 0, 0) = input[0];
    (*this)(0, 0, 1, 1) = input[1];
    // use symmetries
    (*this)(1, 1, 1, 1) = (*this)(0, 0, 0, 0);
    (*this)(1, 1, 0, 0) = (*this)(0, 0, 1, 1);
    (*this)(0, 1, 0, 1) = 0.5 * ((*this)(0, 0, 0, 0) - (*this)(0, 0, 1, 1));
    (*this)(1, 0, 0, 1) = (*this)(0, 1, 0, 1);
    (*this)(0, 1, 1, 0) = (*this)(0, 1, 0, 1);
    (*this)(1, 0, 1, 0) = (*this)(0, 1, 0, 1);
  }
  else
    mooseError("Please provide correct number of inputs for surface RankFourTensorTempl<T> "
               "initialization.");
}

template <typename T>
void
RankFourTensorTempl<T>::fillFromInputVector(const std::vector<T> & input, FillMethod fill_method)
{

  switch (fill_method)
  {
    case antisymmetric:
      fillAntisymmetricFromInputVector(input);
      break;
    case symmetric9:
      fillSymmetric9FromInputVector(input);
      break;
    case symmetric21:
      fillSymmetric21FromInputVector(input);
      break;
    case general_isotropic:
      fillGeneralIsotropicFromInputVector(input);
      break;
    case symmetric_isotropic:
      fillSymmetricIsotropicFromInputVector(input);
      break;
    case symmetric_isotropic_E_nu:
      fillSymmetricIsotropicEandNuFromInputVector(input);
      break;
    case antisymmetric_isotropic:
      fillAntisymmetricIsotropicFromInputVector(input);
      break;
    case axisymmetric_rz:
      fillAxisymmetricRZFromInputVector(input);
      break;
    case general:
      fillGeneralFromInputVector(input);
      break;
    case principal:
      fillPrincipalFromInputVector(input);
      break;
    case orthotropic:
      fillGeneralOrthotropicFromInputVector(input);
      break;
    default:
      mooseError("fillFromInputVector called with unknown fill_method of ", fill_method);
  }
}

template <typename T>
void
RankFourTensorTempl<T>::fillAntisymmetricFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 6)
    mooseError(
        "To use fillAntisymmetricFromInputVector, your input must have size 6.  Yours has size ",
        input.size());

  zero();

  (*this)(0, 1, 0, 1) = input[0]; // B1212
  (*this)(0, 1, 0, 2) = input[1]; // B1213
  (*this)(0, 1, 1, 2) = input[2]; // B1223

  (*this)(0, 2, 0, 2) = input[3]; // B1313
  (*this)(0, 2, 1, 2) = input[4]; // B1323

  (*this)(1, 2, 1, 2) = input[5]; // B2323

  // symmetry on the two pairs
  (*this)(0, 2, 0, 1) = (*this)(0, 1, 0, 2);
  (*this)(1, 2, 0, 1) = (*this)(0, 1, 1, 2);
  (*this)(1, 2, 0, 2) = (*this)(0, 2, 1, 2);
  // have now got the upper parts of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (auto i : make_range(N))
    for (auto j : make_range(N))
    {
      (*this)(0, 1, j, i) = -(*this)(0, 1, i, j);
      (*this)(0, 2, j, i) = -(*this)(0, 2, i, j);
      (*this)(1, 2, j, i) = -(*this)(1, 2, i, j);
    }
  // have now got all of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (auto i : make_range(N))
    for (auto j : make_range(N))
    {
      (*this)(1, 0, i, j) = -(*this)(0, 1, i, j);
      (*this)(2, 0, i, j) = -(*this)(0, 2, i, j);
      (*this)(2, 1, i, j) = -(*this)(1, 2, i, j);
    }
}

template <typename T>
void
RankFourTensorTempl<T>::fillGeneralIsotropicFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 3)
    mooseError("To use fillGeneralIsotropicFromInputVector, your input must have size 3.  Yours "
               "has size ",
               input.size());

  fillGeneralIsotropic(input[0], input[1], input[2]);
}

template <typename T>
void
RankFourTensorTempl<T>::fillGeneralIsotropic(const T & i0, const T & i1, const T & i2)
{
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          (*this)(i, j, k, l) = i0 * Real(i == j) * Real(k == l) +
                                i1 * Real(i == k) * Real(j == l) + i1 * Real(i == l) * Real(j == k);
          for (auto m : make_range(N))
            (*this)(i, j, k, l) +=
                i2 * Real(PermutationTensor::eps(i, j, m)) * Real(PermutationTensor::eps(k, l, m));
        }
}

template <typename T>
void
RankFourTensorTempl<T>::fillAntisymmetricIsotropicFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 1)
    mooseError("To use fillAntisymmetricIsotropicFromInputVector, your input must have size 1. "
               "Yours has size ",
               input.size());

  fillGeneralIsotropic(0.0, 0.0, input[0]);
}

template <typename T>
void
RankFourTensorTempl<T>::fillAntisymmetricIsotropic(const T & i0)
{
  fillGeneralIsotropic(0.0, 0.0, i0);
}

template <typename T>
void
RankFourTensorTempl<T>::fillSymmetricIsotropicFromInputVector(const std::vector<T> & input)
{
  mooseAssert(input.size() == 2,
              "To use fillSymmetricIsotropicFromInputVector, your input must have size 2.");
  fillSymmetricIsotropic(input[0], input[1]);
}

template <typename T>
void
RankFourTensorTempl<T>::fillSymmetricIsotropic(const T & lambda, const T & G)
{
  // clang-format off
  fillSymmetric21FromInputVector(std::array<T,21>
  {{lambda + 2.0 * G, lambda,           lambda,           0.0, 0.0, 0.0,
                      lambda + 2.0 * G, lambda,           0.0, 0.0, 0.0,
                                        lambda + 2.0 * G, 0.0, 0.0, 0.0,
                                                            G, 0.0, 0.0,
                                                                 G, 0.0,
                                                                      G}});
  // clang-format on
}

template <typename T>
void
RankFourTensorTempl<T>::fillSymmetricIsotropicEandNuFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 2)
    mooseError(
        "To use fillSymmetricIsotropicEandNuFromInputVector, your input must have size 2. Yours "
        "has size ",
        input.size());

  fillSymmetricIsotropicEandNu(input[0], input[1]);
}

template <typename T>
void
RankFourTensorTempl<T>::fillSymmetricIsotropicEandNu(const T & E, const T & nu)
{
  // Calculate lambda and the shear modulus from the given young's modulus and poisson's ratio
  const T & lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
  const T & G = E / (2.0 * (1.0 + nu));

  fillSymmetricIsotropic(lambda, G);
}

template <typename T>
void
RankFourTensorTempl<T>::fillAxisymmetricRZFromInputVector(const std::vector<T> & input)
{
  mooseAssert(input.size() == 5,
              "To use fillAxisymmetricRZFromInputVector, your input must have size 5.");

  // C1111  C1122     C1133       0         0         0
  //        C2222  C2233=C1133    0         0         0
  //                  C3333       0         0         0
  //                            C2323       0         0
  //                                   C3131=C2323    0
  //                                                C1212
  // clang-format off
  fillSymmetric21FromInputVector(std::array<T,21>
  {{input[0],input[1],input[2],      0.0,      0.0, 0.0,
             input[0],input[2],      0.0,      0.0, 0.0,
                      input[3],      0.0,      0.0, 0.0,
                                input[4],      0.0, 0.0,
                                          input[4], 0.0,
                                                    (input[0] - input[1]) * 0.5}});
  // clang-format on
}

template <typename T>
void
RankFourTensorTempl<T>::fillGeneralFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 81)
    mooseError("To use fillGeneralFromInputVector, your input must have size 81. Yours has size ",
               input.size());

  for (auto i : make_range(N4))
    _vals[i] = input[i];
}

template <typename T>
void
RankFourTensorTempl<T>::fillPrincipalFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 9)
    mooseError("To use fillPrincipalFromInputVector, your input must have size 9. Yours has size ",
               input.size());

  zero();

  (*this)(0, 0, 0, 0) = input[0];
  (*this)(0, 0, 1, 1) = input[1];
  (*this)(0, 0, 2, 2) = input[2];
  (*this)(1, 1, 0, 0) = input[3];
  (*this)(1, 1, 1, 1) = input[4];
  (*this)(1, 1, 2, 2) = input[5];
  (*this)(2, 2, 0, 0) = input[6];
  (*this)(2, 2, 1, 1) = input[7];
  (*this)(2, 2, 2, 2) = input[8];
}

template <typename T>
void
RankFourTensorTempl<T>::fillGeneralOrthotropicFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 12)
    mooseError("To use fillGeneralOrhotropicFromInputVector, your input must have size 12. Yours "
               "has size ",
               input.size());

  const T & Ea = input[0];
  const T & Eb = input[1];
  const T & Ec = input[2];
  const T & Gab = input[3];
  const T & Gbc = input[4];
  const T & Gca = input[5];
  const T & nuba = input[6];
  const T & nuca = input[7];
  const T & nucb = input[8];
  const T & nuab = input[9];
  const T & nuac = input[10];
  const T & nubc = input[11];

  // Input must satisfy constraints.
  bool preserve_symmetry = MooseUtils::absoluteFuzzyEqual(nuab * Eb, nuba * Ea) &&
                           MooseUtils::absoluteFuzzyEqual(nuca * Ea, nuac * Ec) &&
                           MooseUtils::absoluteFuzzyEqual(nubc * Ec, nucb * Eb);

  if (!preserve_symmetry)
    mooseError("Orthotropic elasticity tensor input is not consistent with symmetry requirements. "
               "Check input for accuracy");

  unsigned int ntens = N * (N + 1) / 2;

  std::vector<T> mat;
  mat.assign(ntens * ntens, 0);

  T k = 1 - nuab * nuba - nubc * nucb - nuca * nuac - nuab * nubc * nuca - nuba * nucb * nuac;

  bool is_positive_definite =
      (k > 0) && (1 - nubc * nucb) > 0 && (1 - nuac * nuca) > 0 && (1 - nuab * nuba) > 0;
  if (!is_positive_definite)
    mooseError("Orthotropic elasticity tensor input is not positive definite. Check input for "
               "accuracy");

  mat[0] = Ea * (1 - nubc * nucb) / k;
  mat[1] = Ea * (nubc * nuca + nuba) / k;
  mat[2] = Ea * (nuba * nucb + nuca) / k;

  mat[6] = Eb * (nuac * nucb + nuab) / k;
  mat[7] = Eb * (1 - nuac * nuca) / k;
  mat[8] = Eb * (nuab * nuca + nucb) / k;

  mat[12] = Ec * (nuab * nubc + nuac) / k;
  mat[13] = Ec * (nuac * nuba + nubc) / k;
  mat[14] = Ec * (1 - nuab * nuba) / k;

  mat[21] = 2 * Gab;
  mat[28] = 2 * Gca;
  mat[35] = 2 * Gbc;

  // Switching from Voigt to fourth order tensor
  // Copied from existing code (invSymm)
  int nskip = N - 1;

  unsigned int index = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          if (i == j)
            (*this)._vals[index] =
                k == l ? mat[i * ntens + k] : mat[i * ntens + k + nskip + l] / 2.0;
          else
            (*this)._vals[index] = k == l ? mat[(nskip + i + j) * ntens + k]
                                          : mat[(nskip + i + j) * ntens + k + nskip + l] / 2.0;
          index++;
        }
}

template <typename T>
RankTwoTensorTempl<T>
RankFourTensorTempl<T>::innerProductTranspose(const RankTwoTensorTempl<T> & b) const
{
  RankTwoTensorTempl<T> result;

  unsigned int index = 0;
  for (unsigned int ij = 0; ij < N2; ++ij)
  {
    T bb = b._coords[ij];
    for (unsigned int kl = 0; kl < N2; ++kl)
      result._coords[kl] += _vals[index++] * bb;
  }

  return result;
}

template <typename T>
T
RankFourTensorTempl<T>::contractionIj(unsigned int i,
                                      unsigned int j,
                                      const RankTwoTensorTempl<T> & M) const
{
  T val = 0;
  for (unsigned int k = 0; k < N; k++)
    for (unsigned int l = 0; l < N; l++)
      val += (*this)(i, j, k, l) * M(k, l);

  return val;
}

template <typename T>
T
RankFourTensorTempl<T>::contractionKl(unsigned int k,
                                      unsigned int l,
                                      const RankTwoTensorTempl<T> & M) const
{
  T val = 0;
  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      val += (*this)(i, j, k, l) * M(i, j);

  return val;
}

template <typename T>
T
RankFourTensorTempl<T>::sum3x3() const
{
  // used in the volumetric locking correction
  T sum = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      sum += (*this)(i, i, j, j);
  return sum;
}

template <typename T>
VectorValue<T>
RankFourTensorTempl<T>::sum3x1() const
{
  // used for volumetric locking correction
  VectorValue<T> a(3);
  a(0) = (*this)(0, 0, 0, 0) + (*this)(0, 0, 1, 1) + (*this)(0, 0, 2, 2); // C0000 + C0011 + C0022
  a(1) = (*this)(1, 1, 0, 0) + (*this)(1, 1, 1, 1) + (*this)(1, 1, 2, 2); // C1100 + C1111 + C1122
  a(2) = (*this)(2, 2, 0, 0) + (*this)(2, 2, 1, 1) + (*this)(2, 2, 2, 2); // C2200 + C2211 + C2222
  return a;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::tripleProductJkl(const RankTwoTensorTempl<T> & A,
                                         const RankTwoTensorTempl<T> & B,
                                         const RankTwoTensorTempl<T> & C) const
{
  RankFourTensorTempl<T> R;
  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            for (unsigned int n = 0; n < N; n++)
              for (unsigned int t = 0; t < N; t++)
                R(i, j, k, l) += (*this)(i, m, n, t) * A(j, m) * B(k, n) * C(l, t);

  return R;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::tripleProductIkl(const RankTwoTensorTempl<T> & A,
                                         const RankTwoTensorTempl<T> & B,
                                         const RankTwoTensorTempl<T> & C) const
{
  RankFourTensorTempl<T> R;
  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            for (unsigned int n = 0; n < N; n++)
              for (unsigned int t = 0; t < N; t++)
                R(i, j, k, l) += (*this)(m, j, n, t) * A(i, m) * B(k, n) * C(l, t);

  return R;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::tripleProductIjl(const RankTwoTensorTempl<T> & A,
                                         const RankTwoTensorTempl<T> & B,
                                         const RankTwoTensorTempl<T> & C) const
{
  RankFourTensorTempl<T> R;
  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            for (unsigned int n = 0; n < N; n++)
              for (unsigned int t = 0; t < N; t++)
                R(i, j, k, l) += (*this)(m, n, k, t) * A(i, m) * B(j, n) * C(l, t);

  return R;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::tripleProductIjk(const RankTwoTensorTempl<T> & A,
                                         const RankTwoTensorTempl<T> & B,
                                         const RankTwoTensorTempl<T> & C) const
{
  RankFourTensorTempl<T> R;
  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            for (unsigned int n = 0; n < N; n++)
              for (unsigned int t = 0; t < N; t++)
                R(i, j, k, l) += (*this)(m, n, t, l) * A(i, m) * B(j, n) * C(k, t);

  return R;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::singleProductI(const RankTwoTensorTempl<T> & A) const
{
  RankFourTensorTempl<T> R;

  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            R(i, j, k, l) += (*this)(m, j, k, l) * A(i, m);

  return R;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::singleProductJ(const RankTwoTensorTempl<T> & A) const
{
  RankFourTensorTempl<T> R;

  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            R(i, j, k, l) += (*this)(i, m, k, l) * A(j, m);

  return R;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::singleProductK(const RankTwoTensorTempl<T> & A) const
{
  RankFourTensorTempl<T> R;

  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            R(i, j, k, l) += (*this)(i, j, m, l) * A(k, m);

  return R;
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::singleProductL(const RankTwoTensorTempl<T> & A) const
{
  RankFourTensorTempl<T> R;

  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      for (unsigned int k = 0; k < N; k++)
        for (unsigned int l = 0; l < N; l++)
          for (unsigned int m = 0; m < N; m++)
            R(i, j, k, l) += (*this)(i, j, k, m) * A(l, m);

  return R;
}

template <typename T>
bool
RankFourTensorTempl<T>::isSymmetric() const
{
  for (auto i : make_range(1u, N))
    for (auto j : make_range(i))
      for (auto k : make_range(1u, N))
        for (auto l : make_range(k))
        {
          // minor symmetries
          if ((*this)(i, j, k, l) != (*this)(j, i, k, l) ||
              (*this)(i, j, k, l) != (*this)(i, j, l, k))
            return false;

          // major symmetry
          if ((*this)(i, j, k, l) != (*this)(k, l, i, j))
            return false;
        }
  return true;
}

template <typename T>
bool
RankFourTensorTempl<T>::isIsotropic() const
{
  // prerequisite is symmetry
  if (!isSymmetric())
    return false;

  // inspect shear components
  const T & mu = (*this)(0, 1, 0, 1);
  // ...diagonal
  if ((*this)(1, 2, 1, 2) != mu || (*this)(2, 0, 2, 0) != mu)
    return false;
  // ...off-diagonal
  if ((*this)(2, 0, 1, 2) != 0.0 || (*this)(0, 1, 1, 2) != 0.0 || (*this)(0, 1, 2, 0) != 0.0)
    return false;

  // off diagonal blocks in Voigt
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      if (_vals[i * (N3 + N2) + ((j + 1) % N) * N + (j + 2) % N] != 0.0)
        return false;

  // top left block
  const T & K1 = (*this)(0, 0, 0, 0);
  const T & K2 = (*this)(0, 0, 1, 1);
  if (!MooseUtils::relativeFuzzyEqual(K1 - 4.0 * mu / 3.0, K2 + 2.0 * mu / 3.0))
    return false;
  if ((*this)(1, 1, 1, 1) != K1 || (*this)(2, 2, 2, 2) != K1)
    return false;
  for (auto i : make_range(1u, N))
    for (auto j : make_range(i))
      if ((*this)(i, i, j, j) != K2)
        return false;

  return true;
}
