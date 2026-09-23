//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTypes.h"

#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

#include "gtest_include.h"

// Real6 and Real66 exist so that a batch of NEML2 SR2 or SSR4 values can be consumed by
// reinterpreting its storage, which requires their component order and Mandel factors to agree
// exactly with SymmetricRankTwoTensor and SymmetricRankFourTensor. These tests pin that agreement
// against those tensors rather than against a restatement of the convention.

namespace
{

/// An asymmetric tensor, so that symmetrization is actually exercised
Moose::Kokkos::Real33
sampleGradient()
{
  Moose::Kokkos::Real33 grad;

  grad(0, 0) = 1.5;
  grad(1, 1) = -2.25;
  grad(2, 2) = 0.75;
  grad(1, 2) = 0.5;
  grad(2, 1) = 0.125;
  grad(0, 2) = -0.375;
  grad(2, 0) = 0.25;
  grad(0, 1) = 0.625;
  grad(1, 0) = -0.125;

  return grad;
}

} // namespace

TEST(KokkosMandelTypesTest, storageMatchesSymmetricRankTwoTensor)
{
  const auto grad = sampleGradient();

  RankTwoTensor full;
  for (const auto i : make_range(3u))
    for (const auto j : make_range(3u))
      full(i, j) = grad(i, j);

  const Moose::Kokkos::Real6 mandel(grad);
  const SymmetricRankTwoTensor reference(full);

  // Component-by-component equality of the stored values, which is what makes reinterpreting
  // NEML2's storage valid
  for (const auto i : make_range(6u))
    EXPECT_DOUBLE_EQ(mandel(i), reference(i));
}

TEST(KokkosMandelTypesTest, fullUndoesTheMandelFactors)
{
  const auto grad = sampleGradient();
  const Moose::Kokkos::Real6 mandel(grad);
  const auto expanded = mandel.full();

  // Expanding recovers the symmetric part of the original gradient
  for (const auto i : make_range(3u))
    for (const auto j : make_range(3u))
      EXPECT_DOUBLE_EQ(expanded(i, j), 0.5 * (grad(i, j) + grad(j, i)));
}

TEST(KokkosMandelTypesTest, contractionAgreesWithTheDenseForm)
{
  const Real lambda = 60.5;
  const Real shear_modulus = 25.9;

  // The same isotropic tensor in both representations, each filled by the non-Kokkos tensor it
  // mirrors, so the comparison does not depend on this test getting the fill right
  RankFourTensor dense_reference;
  dense_reference.fillSymmetricIsotropic(lambda, shear_modulus);
  SymmetricRankFourTensor mandel_reference;
  mandel_reference.fillSymmetricIsotropic(lambda, shear_modulus);

  Moose::Kokkos::Real3333 dense;
  for (const auto i : make_range(3u))
    for (const auto j : make_range(3u))
      for (const auto k : make_range(3u))
        for (const auto l : make_range(3u))
          dense(i, j, k, l) = dense_reference(i, j, k, l);

  Moose::Kokkos::Real66 mandel;
  for (const auto i : make_range(6u))
    for (const auto j : make_range(6u))
      mandel(i, j) = mandel_reference(i, j);

  const auto grad = sampleGradient();

  // Dense: contract the full tangent with the unsymmetrized gradient, as the kernel does
  const auto dense_stress = dense * grad;

  // Mandel: the same contraction as a six by six matrix-vector product
  const auto mandel_stress = (mandel * Moose::Kokkos::Real6(grad)).full();

  for (const auto i : make_range(3u))
    for (const auto j : make_range(3u))
      EXPECT_NEAR(mandel_stress(i, j), dense_stress(i, j), 1e-12 * std::abs(dense_stress(0, 0)));
}
