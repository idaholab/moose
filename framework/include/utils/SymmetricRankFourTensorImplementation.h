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
#include "RankFourTensor.h"
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
  return MooseEnum("symmetric9 symmetric21  symmetric_isotropic symmetric_isotropic_E_nu  "
                   "axisymmetric_rz principal orthotropic");
}

template <typename T>
SymmetricRankFourTensorTempl<T>::SymmetricRankFourTensorTempl()
{
  mooseAssert(Ndim == 3,
              "SymmetricRankFourTensorTempl<T> is designed to only work in 3 dimensions.");
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
      for (const auto i : make_range(Ndim))
        (*this)(i, i) = 1.0;
      break;

    case initIdentitySymmetricFour:
      zero();
      for (const auto i : make_range(N))
        (*this)(i, i) = 1.0;
      break;

    default:
      mooseError("Unknown SymmetricRankFourTensorTempl<T> initialization pattern.");
  }
}

template <typename T>
SymmetricRankFourTensorTempl<T>::operator RankFourTensorTempl<T>()
{
  // Full tensor indices in the Mandel/Voigt representation
  static constexpr unsigned int g[6][6][4] = {
      {{1, 1, 1, 1}, {1, 1, 2, 2}, {1, 1, 3, 3}, {1, 1, 2, 3}, {1, 1, 1, 3}, {1, 1, 1, 2}},
      {{2, 2, 1, 1}, {2, 2, 2, 2}, {2, 2, 3, 3}, {2, 2, 2, 3}, {2, 2, 1, 3}, {2, 2, 1, 2}},
      {{3, 3, 1, 1}, {3, 3, 2, 2}, {3, 3, 3, 3}, {3, 3, 2, 3}, {3, 3, 1, 3}, {3, 3, 1, 2}},
      {{2, 3, 1, 1}, {2, 3, 2, 2}, {2, 3, 3, 3}, {2, 3, 2, 3}, {2, 3, 1, 3}, {2, 3, 1, 2}},
      {{1, 3, 1, 1}, {1, 3, 2, 2}, {1, 3, 3, 3}, {1, 3, 2, 3}, {1, 3, 1, 3}, {1, 3, 1, 2}},
      {{1, 2, 1, 1}, {1, 2, 2, 2}, {1, 2, 3, 3}, {1, 2, 2, 3}, {1, 2, 1, 3}, {1, 2, 1, 2}}};

  auto & q = *this;
  RankFourTensorTempl<T> r;
  for (const auto a : make_range(N))
    for (const auto b : make_range(N))
    {
      const auto i = g[a][b][0] - 1;
      const auto j = g[a][b][1] - 1;
      const auto k = g[a][b][2] - 1;
      const auto l = g[a][b][3] - 1;

      // Rijkl = Rjikl = Rijlk = Rjilk
      r(i, j, k, l) = q(a, b) / mandelFactor(a, b);
      r(j, i, k, l) = q(a, b) / mandelFactor(a, b);
      r(i, j, l, k) = q(a, b) / mandelFactor(a, b);
      r(j, i, l, k) = q(a, b) / mandelFactor(a, b);
    }

  return r;
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
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::rotationMatrix(const TypeTensor<T> & R)
{
  SymmetricRankFourTensorTempl<T> M(initNone);
  const static std::array<std::size_t, 3> a = {{1, 0, 0}};
  const static std::array<std::size_t, 3> b = {{2, 2, 1}};
  for (std::size_t i = 0; i < 3; ++i)
    for (std::size_t j = 0; j < 3; ++j)
    {
      M(i, j) = R(i, j) * R(i, j);
      M(i + 3, j) = MathUtils::sqrt2 * R((i + 1) % 3, j) * R((i + 2) % 3, j);
      M(j, i + 3) = MathUtils::sqrt2 * R(j, (i + 1) % 3) * R(j, (i + 2) % 3);
      M(i + 3, j + 3) = R(a[i], a[j]) * R(b[i], b[j]) + R(a[i], b[j]) * R(b[i], a[j]);
    }
  return M;
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::rotate(const TypeTensor<T> & R)
{
  // build 6x6 rotation matrix
  auto M = SymmetricRankFourTensorTempl<T>::rotationMatrix(R);

  // rotate tensor
  (*this) = M * (*this) * M.transposeMajor();
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
  for (const auto i : make_range(N2))
    _vals[i] *= a;
  return *this;
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator/=(const T & a)
{
  for (const auto i : make_range(N2))
    _vals[i] /= a;
  return *this;
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator+=(const SymmetricRankFourTensorTempl<T> & a)
{
  for (const auto i : make_range(N2))
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
  for (const auto i : make_range(N2))
    result._vals[i] = _vals[i] + b._vals[i];
  return result;
}

template <typename T>
SymmetricRankFourTensorTempl<T> &
SymmetricRankFourTensorTempl<T>::operator-=(const SymmetricRankFourTensorTempl<T> & a)
{
  for (const auto i : make_range(N2))
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
  for (const auto i : make_range(N2))
    result._vals[i] = _vals[i] - b._vals[i];
  return result;
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::operator-() const
{
  SymmetricRankFourTensorTempl<T> result;
  for (const auto i : make_range(N2))
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

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      for (const auto p : make_range(N))
        result(i, j) += (*this)(i, p) * b(p, j);

  return result;
}

template <typename T>
T
SymmetricRankFourTensorTempl<T>::L2norm() const
{
  T l2 = Utility::pow<2>(_vals[0]);
  for (const auto i : make_range(1u, N2))
    l2 += Utility::pow<2>(_vals[i]);
  return std::sqrt(l2);
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::print(std::ostream & stm) const
{
  for (const auto i : make_range(N))
  {
    for (const auto j : make_range(N))
      stm << std::setw(15) << _vals[i * N + j] << " ";
    stm << '\n';
  }
  stm << std::flush;
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::printReal(std::ostream & stm) const
{
  for (const auto i : make_range(N))
  {
    for (const auto j : make_range(N))
      stm << std::setw(15) << MetaPhysicL::raw_value(_vals[i * N + j]) << " ";
    stm << '\n';
  }
  stm << std::flush;
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::transposeMajor() const
{
  std::size_t index = 0;
  SymmetricRankFourTensorTempl<T> ret;
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      ret._vals[index++] = _vals[i + N * j];
  return ret;
}

template <typename T>
void
SymmetricRankFourTensorTempl<T>::fillFromInputVector(const std::vector<T> & input,
                                                     FillMethod fill_method)
{

  switch (fill_method)
  {
    case symmetric9:
      fillSymmetric9FromInputVector(input);
      break;
    case symmetric21:
      fillSymmetric21FromInputVector(input);
      break;
    case symmetric_isotropic:
      fillSymmetricIsotropicFromInputVector(input);
      break;
    case symmetric_isotropic_E_nu:
      fillSymmetricIsotropicEandNuFromInputVector(input);
      break;
    case axisymmetric_rz:
      fillAxisymmetricRZFromInputVector(input);
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
  mooseAssert(LIBMESH_DIM == 3, "This method assumes LIBMESH_DIM == 3");
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
  bool preserve_symmetry = MooseUtils::relativeFuzzyEqual(nuab * Eb, nuba * Ea) &&
                           MooseUtils::relativeFuzzyEqual(nuca * Ea, nuac * Ec) &&
                           MooseUtils::relativeFuzzyEqual(nubc * Ec, nucb * Eb);

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

  _vals[0] = Ea * (1 - nubc * nucb) / k;
  _vals[1] = Ea * (nubc * nuca + nuba) / k;
  _vals[2] = Ea * (nuba * nucb + nuca) / k;

  _vals[6] = Eb * (nuac * nucb + nuab) / k;
  _vals[7] = Eb * (1 - nuac * nuca) / k;
  _vals[8] = Eb * (nuab * nuca + nucb) / k;

  _vals[12] = Ec * (nuab * nubc + nuac) / k;
  _vals[13] = Ec * (nuac * nuba + nubc) / k;
  _vals[14] = Ec * (1 - nuab * nuba) / k;

  _vals[21] = 2 * Gbc;
  _vals[28] = 2 * Gca;
  _vals[35] = 2 * Gab;
}

template <typename T>
T
SymmetricRankFourTensorTempl<T>::sum3x3() const
{
  mooseAssert(LIBMESH_DIM == 3, "This method assumes LIBMESH_DIM == 3");
  // summation of Ciijj used in the volumetric locking correction
  T sum = 0;
  for (const auto i : make_range(3))
    for (const auto j : make_range(3))
      sum += (*this)(i, j);
  return sum;
}

template <typename T>
VectorValue<T>
SymmetricRankFourTensorTempl<T>::sum3x1() const
{
  mooseAssert(LIBMESH_DIM == 3, "This method assumes LIBMESH_DIM == 3");
  // used for volumetric locking correction
  return VectorValue<T>(_vals[0] + _vals[1] + _vals[2],
                        _vals[6] + _vals[7] + _vals[8],
                        _vals[12] + _vals[13] + _vals[14]);
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
  for (const auto i : make_range(3))
    for (const auto j : make_range(3))
      if (_vals[3 + i + N * j] != 0.0)
        return false;

  // top left block
  const T & K1 = _vals[0];
  const T & K2 = _vals[1];
  if (!MooseUtils::relativeFuzzyEqual(K1 - 2.0 * mu / 3.0, K2 + mu / 3.0))
    return false;
  if (_vals[7] != K1 || _vals[14] != K1)
    return false;

  for (const auto i : make_range(1, 3))
    for (const auto j : make_range(i))
      if (_vals[i + N * j] != K2)
        return false;

  return true;
}
