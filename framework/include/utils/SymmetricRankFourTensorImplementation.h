//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetricRankFourTensor.h"

// MOOSE includes
#include "SymmetricRankTwoTensor.h"
#include "RankThreeTensor.h"
#include "MooseEnum.h"
#include "MooseException.h"
#include "MooseUtils.h"
#include "MatrixTools.h"
#include "PermutationTensor.h"

#include "libmesh/utility.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

// C++ includes
#include <iomanip>
#include <ostream>

namespace MathUtils
{
template <>
void mooseSetToZero<SymmetricRankFourTensorTempl<Real>>(SymmetricRankFourTensorTempl<Real> & v);
template <>
void
mooseSetToZero<SymmetricRankFourTensorTempl<DualReal>>(SymmetricRankFourTensorTempl<DualReal> & v);
}

template <typename T>
MooseEnum
SymmetricRankFourTensorTempl<T>::fillMethodEnum()
{
  return MooseEnum("antisymmetric symmetric9 symmetric21 general_isotropic symmetric_isotropic "
                   "symmetric_isotropic_E_nu antisymmetric_isotropic axisymmetric_rz general "
                   "principal orthotropic");
}

template <typename T>
SymmetricRankFourTensorTempl<T>::SymmetricRankFourTensorTempl()
{
  mooseAssert(N == 6, "SymmetricRankFourTensorTempl<T> is currently only tested for 3 dimensions.");
  zero();
}

template <typename T>
SymmetricRankFourTensorTempl<T>::SymmetricRankFourTensorTempl(const InitMethod init)
{
  switch (init)
  {
    case initNone:
      break;

    case initIdentity:
      zero();
      for (unsigned int i = 0; i < N; ++i)
        _vals[i * 7] = 1.0;
      break;

    case initIdentityFour:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
        {
          _vals[i + N * j] = 1.0;
          _vals[i + N * j + 3] = 0.0;
          _vals[i + N * (j + 3)] = 0.0;
          _vals[i + N * (j + 3) + 3] = 2.0 * Real(i == j);
        }
      break;

      // case initIdentitySymmetricFour:
      //   for (unsigned int i = 0; i < N; ++i)
      //     for (unsigned int j = 0; j < N; ++j)
      //       for (unsigned int k = 0; k < N; ++k)
      //         for (unsigned int l = 0; l < N; ++l)
      //           _vals[index++] = 0.5 * Real(i == k && j == l) + 0.5 * Real(i == l && j == k);
      //   break;

    default:
      mooseError("Unknown SymmetricRankFourTensorTempl<T> initialization pattern.");
  }
}

template <typename T>
SymmetricRankFourTensorTempl<T>::SymmetricRankFourTensorTempl(const std::vector<T> & input,
                                                              FillMethod fill_method)
{
  fillFromInputVector(input, fill_method);
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::zero()
{
  std::fill(_vals.begin(), _vals.end(), 0.0);
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator=(const SymmetricRankFourTensorTempl<T> & a)
{
  std::copy(a._vals.begin(), a._vals.end(), _vals.begin());
  return *this;
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator*=(const T & a)
{
  for (std::size_t i = 0; i < N2; ++i)
    _vals[i] *= a;
  return *this;
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator/=(const T & a)
{
  for (std::size_t i = 0; i < N2; ++i)
    _vals[i] /= a;
  return *this;
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator+=(const SymmetricRankFourTensorTempl<T> & a)
{
  for (std::size_t i = 0; i < N2; ++i)
    _vals[i] += a._vals[i];
  return *this;
}

template <typename T>
template <typename T2>
auto
SymmetricRankFourTensorTempl<T>::operator+(const SymmetricRankFourTensorTempl<T2> & b) const
    -> SymmetricRankFourTensorTempl<decltype(T() + T2())>
{
  SymmetricRankFourTensorTempl<decltype(T() + T2())> result;
  for (std::size_t i = 0; i < N2; ++i)
    result._vals[i] = _vals[i] + b._vals[i];
  return result;
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator-=(const SymmetricRankFourTensorTempl<T> & a)
{
  for (std::size_t i = 0; i < N2; ++i)
    _vals[i] -= a._vals[i];
  return *this;
}

template <typename T>
template <typename T2>
auto
SymmetricRankFourTensorTempl<T>::operator-(const SymmetricRankFourTensorTempl<T2> & b) const
    -> SymmetricRankFourTensorTempl<decltype(T() - T2())>
{
  SymmetricRankFourTensorTempl<decltype(T() - T2())> result;
  for (std::size_t i = 0; i < N2; ++i)
    result._vals[i] = _vals[i] - b._vals[i];
  return result;
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::operator-() const
{
  SymmetricRankFourTensorTempl<T> result;
  for (std::size_t i = 0; i < N2; ++i)
    result._vals[i] = -_vals[i];
  return result;
}

template <typename T>
template <typename T2>
auto
SymmetricRankFourTensorTempl<T>::operator*(const SymmetricRankFourTensorTempl<T2> & b) const
    -> SymmetricRankFourTensorTempl<decltype(T() * T2())>
{
  typedef decltype(T() * T2()) ValueType;
  SymmetricRankFourTensorTempl<ValueType> result;

  for (std::size_t i = 0; i < N; ++i)
    for (std::size_t j = 0; j < N; ++j)
      for (std::size_t k = 0; k < N; ++k)
        result._vals[i + N * j] += _vals[i + N * k] * b._vals[k + N * j];

  return result;
}

template <typename T>
T
SymmetricRankFourTensorTempl<T>::L2norm() const
{
  T l2 = Utility::pow<2>(_vals[0]);
  for (std::size_t i = 1; i < N2; ++i)
    l2 += Utility::pow<2>(_vals[i]);
  return std::sqrt(l2);
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::invSymm() const
{
  mooseError("The invSymm operation calls to LAPACK and only supports plain Real type tensors.");
}

template <>
SymmetricRankFourTensorTempl<Real>
SymmetricRankFourTensorTempl<Real>::invSymm() const
{
  std::vector<PetscScalar> buf(_vals.begin(), _vals.end());

  // use LAPACK to find the inverse
  MatrixTools::inverse(buf, 6);

  SymmetricRankFourTensorTempl<Real> result(initNone);
  std::copy(buf.begin(), buf.end(), result._vals.begin());
  return result;
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::rotate(const TypeTensor<T> & /*R*/)
{
  mooseError("Not implemented yet (use bond-like matrix)");
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::print(std::ostream & stm) const
{
  std::size_t index = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    for (unsigned int j = 0; j < N; ++j)
      stm << std::setw(15) << _vals[index++] << " ";
    stm << '\n';
  }
  stm << std::flush;
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::transposeMajor() const
{
  mooseError("Not implemented yet");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::transposeIj() const
{
  mooseError("Not implemented yet");
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::surfaceFillFromInputVector(const std::vector<T> & /*input*/)
{
  mooseError("Not implemented yet");
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillFromInputVector(const std::vector<T> & input,
                                                     FillMethod fill_method)
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
SymmetricRankFourTensorTempl<T>::fillAntisymmetricFromInputVector(const std::vector<T> & /*input*/)
{
  mooseError("Not possible");
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillGeneralIsotropicFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 3)
    mooseError("To use fillGeneralIsotropicFromInputVector, your input must have size 3.  Yours "
               "has size ",
               input.size());

  fillGeneralIsotropic(input[0], input[1], input[2]);
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillGeneralIsotropic(const T & i0, const T & i1, const T & /*i2*/)
{

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      _vals[i + j * N] = i0;

  _vals[21] = _vals[28] = _vals[35] = i1;

  //???
  // for (unsigned int m = 0; m < N; ++m)
  //   (*this)(i, j, k, l) +=
  //       i2 * Real(PermutationTensor::eps(i, j, m)) * Real(PermutationTensor::eps(k, l, m));
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillAntisymmetricIsotropicFromInputVector(
    const std::vector<T> & input)
{
  if (input.size() != 1)
    mooseError("To use fillAntisymmetricIsotropicFromInputVector, your input must have size 1. "
               "Yours has size ",
               input.size());

  fillGeneralIsotropic(0.0, 0.0, input[0]);
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillAntisymmetricIsotropic(const T & i0)
{
  fillGeneralIsotropic(0.0, 0.0, i0);
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillSymmetricIsotropicFromInputVector(const std::vector<T> & input)
{
  mooseAssert(input.size() == 2,
              "To use fillSymmetricIsotropicFromInputVector, your input must have size 2.");
  fillSymmetricIsotropic(input[0], input[1]);
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillSymmetricIsotropic(const T & lambda, const T & G)
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
SymmetricRankFourTensorTempl<T>::fillSymmetricIsotropicEandNuFromInputVector(
    const std::vector<T> & input)
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
SymmetricRankFourTensorTempl<T>::fillSymmetricIsotropicEandNu(const T & E, const T & nu)
{
  // Calculate lambda and the shear modulus from the given young's modulus and poisson's ratio
  const T & lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
  const T & G = E / (2.0 * (1.0 + nu));

  fillSymmetricIsotropic(lambda, G);
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillAxisymmetricRZFromInputVector(const std::vector<T> & input)
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
SymmetricRankFourTensorTempl<T>::fillGeneralFromInputVector(const std::vector<T> & /*input*/)
{
  mooseError("Not implemented");
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillPrincipalFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 9)
    mooseError("To use fillPrincipalFromInputVector, your input must have size 9. Yours has size ",
               input.size());

  zero();

  // top left block
  _vals[0] = input[0];
  _vals[1] = input[1];
  _vals[2] = input[2];
  _vals[6] = input[3];
  _vals[7] = input[4];
  _vals[8] = input[5];
  _vals[12] = input[6];
  _vals[13] = input[7];
  _vals[14] = input[8];
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillGeneralOrthotropicFromInputVector(const std::vector<T> & input)
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

  zero();
  T k = 1 - nuab * nuba - nubc * nucb - nuca * nuac - nuab * nubc * nuca - nuba * nucb * nuac;

  bool is_positive_definite =
      (k > 0) && (1 - nubc * nucb) > 0 && (1 - nuac * nuca) > 0 && (1 - nuab * nuba) > 0;
  if (!is_positive_definite)
    mooseError("Orthotropic elasticity tensor input is not positive definite. Check input for "
               "accuracy");

  // double check if this is Mandel (probably not)
  _vals[0] = Ea * (1 - nubc * nucb) / k;
  _vals[1] = Ea * (nubc * nuca + nuba) / k;
  _vals[2] = Ea * (nuba * nucb + nuca) / k;

  _vals[6] = Eb * (nuac * nucb + nuab) / k;
  _vals[7] = Eb * (1 - nuac * nuca) / k;
  _vals[8] = Eb * (nuab * nuca + nucb) / k;

  _vals[12] = Ec * (nuab * nubc + nuac) / k;
  _vals[13] = Ec * (nuac * nuba + nubc) / k;
  _vals[14] = Ec * (1 - nuab * nuba) / k;

  _vals[21] = 2 * Gab;
  _vals[28] = 2 * Gca;
  _vals[35] = 2 * Gbc;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankFourTensorTempl<T>::innerProductTranspose(
    const SymmetricRankTwoTensorTempl<T> & b) const
{
  SymmetricRankTwoTensorTempl<T> result;

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
SymmetricRankFourTensorTempl<T>::sum3x3() const
{
  // summation of Ciijj for i and j ranging from 0 to 2 - used in the volumetric locking
  // correction
  return _vals[0] + _vals[1] + _vals[2] + _vals[6] + _vals[7] + _vals[8] + _vals[12] + _vals[13] +
         _vals[14];
  ;
}

template <typename T>
VectorValue<T>
SymmetricRankFourTensorTempl<T>::sum3x1() const
{
  // used for volumetric locking correction
  VectorValue<T> a(3);
  a(0) = _vals[0] + _vals[1] + _vals[2];    // C0000 + C0011 + C0022
  a(1) = _vals[6] + _vals[7] + _vals[8];    // C1100 + C1111 + C1122
  a(2) = _vals[12] + _vals[13] + _vals[14]; // C2200 + C2211 + C2222
  return a;
}

template <typename T>
bool
SymmetricRankFourTensorTempl<T>::isSymmetric() const
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      // major symmetry
      if (_vals[i + N * j] != _vals[N * i + j])
        return false;
  return true;
}

template <typename T>
bool
SymmetricRankFourTensorTempl<T>::isIsotropic() const
{
  // prerequisite is symmetry
  if (!isSymmetric())
    return false;

  // inspect shear components
  const T & mu = _vals[35];

  // ...diagonal
  if (_vals[28] != mu || _vals[21] != mu)
    return false;

  // ...off-diagonal
  if (_vals[22] != 0.0 || _vals[23] != 0.0 || _vals[29] != 0.0)
    return false;

  // off diagonal blocks in Voigt
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      if (_vals[3 + i + N * j] != 0.0)
        return false;

  // top left block
  const T & K1 = _vals[0];
  const T & K2 = _vals[1];
  if (!MooseUtils::relativeFuzzyEqual(K1 - 4.0 * mu / 3.0, K2 + 2.0 * mu / 3.0))
    return false;
  if (_vals[7] != K1 || _vals[14] != K1)
    return false;

  for (unsigned int i = 1; i < N; ++i)
    for (unsigned int j = 0; j < i; ++j)
      if (_vals[i + N * j] != K2)
        return false;

  return true;
}
