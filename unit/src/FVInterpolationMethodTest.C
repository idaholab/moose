//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObjectUnitTest.h"

#include "FVAdvectedMinmodWeightBased.h"
#include "FVAdvectedUpwind.h"
#include "FVAdvectedVanLeerWeightBased.h"
#include "FVAdvectedVenkatakrishnanDeferredCorrection.h"
#include "FVGeometricAverage.h"
#include "FVHarmonicAverage.h"
#include "MathFVUtils.h"

#include "gtest/gtest.h"

#include <cmath>

class FVInterpolationMethodTest : public MooseObjectUnitTest
{
public:
  FVInterpolationMethodTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    _mesh->buildFiniteVolumeInfo();
    for (const auto * fi : _mesh->faceInfo())
      if (fi->neighborPtr())
      {
        _internal_face = fi;
        break;
      }

    mooseAssert(_internal_face, "Expected to find at least one internal face in the unit mesh");
  }

protected:
  const FaceInfo * _internal_face = nullptr;
};

TEST_F(FVInterpolationMethodTest, geometricAverageMatchesGC)
{
  InputParameters params = _factory.getValidParams("FVGeometricAverage");
  auto & method = addObject<FVGeometricAverage>("FVGeometricAverage", "geom_interp_method", params);

  const Real elem_value = 2.0;
  const Real neighbor_value = 10.0;
  const Real expected =
      _internal_face->gC() * elem_value + (1.0 - _internal_face->gC()) * neighbor_value;

  EXPECT_NEAR(method.interpolate(*_internal_face, elem_value, neighbor_value), expected, 1e-12);
}

TEST_F(FVInterpolationMethodTest, harmonicAverage)
{
  InputParameters params = _factory.getValidParams("FVHarmonicAverage");
  auto & method = addObject<FVHarmonicAverage>("FVHarmonicAverage", "harm_interp_method", params);

  const Real elem_value = 4.0;
  const Real neighbor_value = 1.0;

  const Real gc = _internal_face->gC();
  const Real expected = 1.0 / (gc / elem_value + (1.0 - gc) / neighbor_value);

  EXPECT_NEAR(method.interpolate(*_internal_face, elem_value, neighbor_value), expected, 1e-12);
}

TEST_F(FVInterpolationMethodTest, advectedUpwind)
{
  InputParameters params = _factory.getValidParams("FVAdvectedUpwind");
  auto & method = addObject<FVAdvectedUpwind>("FVAdvectedUpwind", "adv_upwind_method", params);

  const Real elem_value = 2.0;
  const Real neighbor_value = 10.0;

  {
    const Real mass_flux = 1.0;
    const auto contrib = method.advectedInterpolate(
        *_internal_face, elem_value, neighbor_value, nullptr, nullptr, mass_flux);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
    EXPECT_DOUBLE_EQ(contrib.rhs_face_value, 0.0);
    EXPECT_DOUBLE_EQ(method.advectedInterpolateValue(
                         *_internal_face, elem_value, neighbor_value, nullptr, nullptr, mass_flux),
                     elem_value);
  }

  {
    const Real mass_flux = -1.0;
    const auto contrib = method.advectedInterpolate(
        *_internal_face, elem_value, neighbor_value, nullptr, nullptr, mass_flux);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 0.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 1.0);
    EXPECT_DOUBLE_EQ(contrib.rhs_face_value, 0.0);
    EXPECT_DOUBLE_EQ(method.advectedInterpolateValue(
                         *_internal_face, elem_value, neighbor_value, nullptr, nullptr, mass_flux),
                     neighbor_value);
  }
}

TEST_F(FVInterpolationMethodTest, advectedMinmodWeightBasedBlendingFactorZeroIsUpwind)
{
  InputParameters params = _factory.getValidParams("FVAdvectedMinmodWeightBased");
  params.set<Real>("blending_factor") = 0.0;
  auto & method = addObject<FVAdvectedMinmodWeightBased>(
      "FVAdvectedMinmodWeightBased", "adv_minmod_method", params);

  const Real elem_value = 1.0;
  const Real neighbor_value = 2.0;
  const VectorValue<Real> elem_grad(0.1, 0.2, 0.0);
  const VectorValue<Real> neighbor_grad(-0.3, 0.4, 0.0);

  {
    const Real mass_flux = 1.0;
    const auto contrib = method.advectedInterpolate(
        *_internal_face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
  }

  {
    const Real mass_flux = -1.0;
    const auto contrib = method.advectedInterpolate(
        *_internal_face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 0.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 1.0);
  }
}

TEST_F(FVInterpolationMethodTest, advectedMinmodWeightBasedFullBlendYieldsLinearWeights)
{
  InputParameters params = _factory.getValidParams("FVAdvectedMinmodWeightBased");
  params.set<Real>("blending_factor") = 1.0;
  auto & method = addObject<FVAdvectedMinmodWeightBased>(
      "FVAdvectedMinmodWeightBased", "adv_minmod_method_linear", params);

  const Real elem_value = 3.0;
  const Real neighbor_value = 3.0;

  const auto & face = *_internal_face;
  const Real gc = face.gC();
  const Real expected_w_elem = gc;
  const Real expected_w_neighbor = 1.0 - gc;

  // Choose gradients aligned with the dCN direction so that rF's denominator-zero branch yields a
  // large positive r_f (forcing minmod beta = 1).
  const VectorValue<Real> elem_grad = face.dCN();
  const VectorValue<Real> neighbor_grad = -face.dCN();

  {
    const Real mass_flux = 1.0;
    const auto contrib = method.advectedInterpolate(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_NEAR(contrib.weights_matrix.first, expected_w_elem, 1e-12);
    EXPECT_NEAR(contrib.weights_matrix.second, expected_w_neighbor, 1e-12);
  }

  {
    const Real mass_flux = -1.0;
    const auto contrib = method.advectedInterpolate(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_NEAR(contrib.weights_matrix.first, expected_w_elem, 1e-12);
    EXPECT_NEAR(contrib.weights_matrix.second, expected_w_neighbor, 1e-12);
  }
}

TEST_F(FVInterpolationMethodTest, advectedVanLeerWeightBasedClampsToLinearWhenRequested)
{
  const auto & face = *_internal_face;
  const Real gc = face.gC();
  const Real expected_w_elem_linear = gc;
  const Real expected_w_neighbor_linear = 1.0 - gc;

  // So phi_i - phi_j = +/- 1 depending on the mass flux
  const Real elem_value = 1.0;
  const Real neighbor_value = 2.0;

  // This here makes the limiting active the gradients would result in
  // a change of 1.5 (compared to the 1 difference above)
  const Real grad_scale_clip = 1.5 / (face.dCN() * face.dCN());
  const VectorValue<Real> elem_grad = grad_scale_clip * face.dCN();
  const VectorValue<Real> neighbor_grad = elem_grad;

  InputParameters params_clamped = _factory.getValidParams("FVAdvectedVanLeerWeightBased");
  params_clamped.set<bool>("limit_to_linear") = true;
  params_clamped.set<Real>("blending_factor") = 1.0;
  auto & method_clamped = addObject<FVAdvectedVanLeerWeightBased>(
      "FVAdvectedVanLeerWeightBased", "adv_vanleer_method_clamped", params_clamped);

  InputParameters params_unclamped = _factory.getValidParams("FVAdvectedVanLeerWeightBased");
  params_unclamped.set<bool>("limit_to_linear") = false;
  params_unclamped.set<Real>("blending_factor") = 1.0;
  auto & method_unclamped = addObject<FVAdvectedVanLeerWeightBased>(
      "FVAdvectedVanLeerWeightBased", "adv_vanleer_method_unclamped", params_unclamped);

  auto expected_unclamped = [&](const Real elem_value,
                                const Real neighbor_value,
                                const VectorValue<Real> & elem_grad,
                                const VectorValue<Real> & neighbor_grad,
                                const Real mass_flux)
  {
    const bool upwind_is_elem = mass_flux >= 0.0;

    const Real phi_upwind = upwind_is_elem ? elem_value : neighbor_value;
    const Real phi_downwind = upwind_is_elem ? neighbor_value : elem_value;
    const VectorValue<Real> grad_upwind = upwind_is_elem ? elem_grad : neighbor_grad;
    const Point upwind_to_downwind = upwind_is_elem ? face.dCN() : Point(-face.dCN());

    const auto r_f = Moose::FV::rF(phi_upwind, phi_downwind, grad_upwind, upwind_to_downwind);
    const Real beta = (r_f + abs(r_f)) / (1.0 + std::abs(r_f));

    const Real w_f = upwind_is_elem ? gc : (1.0 - gc);
    const Real g = beta * (1.0 - w_f);

    const Real w_upwind = 1.0 - g;
    const Real w_downwind = g;
    const Real w_elem = upwind_is_elem ? w_upwind : w_downwind;
    const Real w_neighbor = upwind_is_elem ? w_downwind : w_upwind;
    return std::make_pair(w_elem, w_neighbor);
  };

  // This point produces r_f > 1 and therefore beta > 1 with a nonzero value jump across the face,
  // so the unclamped weights become more downwind-biased than linear interpolation.
  // `limit_to_linear = true` should therefore matter here.
  for (const Real mass_flux : {1.0, -1.0})
  {
    const auto contrib_clamped = method_clamped.advectedInterpolate(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_NEAR(contrib_clamped.weights_matrix.first, expected_w_elem_linear, 1e-12);
    EXPECT_NEAR(contrib_clamped.weights_matrix.second, expected_w_neighbor_linear, 1e-12);

    const auto contrib_unclamped = method_unclamped.advectedInterpolate(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    const auto expected =
        expected_unclamped(elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
    EXPECT_NEAR(contrib_unclamped.weights_matrix.first, expected.first, 1e-12);
    EXPECT_NEAR(contrib_unclamped.weights_matrix.second, expected.second, 1e-12);
    EXPECT_GT(
        std::abs(contrib_unclamped.weights_matrix.first - contrib_clamped.weights_matrix.first),
        1e-12);
    EXPECT_GT(
        std::abs(contrib_unclamped.weights_matrix.second - contrib_clamped.weights_matrix.second),
        1e-12);
  }

  // This point produces 0 < beta < 1, so the scheme is already less downwind-biased than linear
  // interpolation and the linear clamp should have no effect.
  const Real elem_value_noclip = 1.0;
  const Real neighbor_value_noclip = 2.0;

  // This gradient will produce a 0.75 change in the value (compared to the 1 we have above)
  const Real grad_scale = 0.75 / (face.dCN() * face.dCN());
  const VectorValue<Real> elem_grad_noclip = grad_scale * face.dCN();
  const VectorValue<Real> neighbor_grad_noclip = elem_grad_noclip;

  for (const Real mass_flux : {1.0, -1.0})
  {
    const auto contrib_clamped = method_clamped.advectedInterpolate(face,
                                                                    elem_value_noclip,
                                                                    neighbor_value_noclip,
                                                                    &elem_grad_noclip,
                                                                    &neighbor_grad_noclip,
                                                                    mass_flux);
    const auto contrib_unclamped = method_unclamped.advectedInterpolate(face,
                                                                        elem_value_noclip,
                                                                        neighbor_value_noclip,
                                                                        &elem_grad_noclip,
                                                                        &neighbor_grad_noclip,
                                                                        mass_flux);
    const auto expected = expected_unclamped(elem_value_noclip,
                                             neighbor_value_noclip,
                                             elem_grad_noclip,
                                             neighbor_grad_noclip,
                                             mass_flux);

    EXPECT_NEAR(contrib_clamped.weights_matrix.first, expected.first, 1e-12);
    EXPECT_NEAR(contrib_clamped.weights_matrix.second, expected.second, 1e-12);
    EXPECT_NEAR(contrib_unclamped.weights_matrix.first, expected.first, 1e-12);
    EXPECT_NEAR(contrib_unclamped.weights_matrix.second, expected.second, 1e-12);
  }
}

TEST_F(FVInterpolationMethodTest, advectedVenkatakrishnanDeferredCorrection)
{
  const auto & face = *_internal_face;

  const Real elem_value = 2.0;
  const Real neighbor_value = 10.0;
  const VectorValue<Real> elem_grad(0.1, 0.2, 0.0);
  const VectorValue<Real> neighbor_grad(-0.3, 0.4, 0.0);
  const Real mass_flux = 1.0;

  {
    InputParameters params = _factory.getValidParams("FVAdvectedVenkatakrishnanDeferredCorrection");
    params.set<Real>("deferred_correction_factor") = 0.0;
    auto & method = addObject<FVAdvectedVenkatakrishnanDeferredCorrection>(
        "FVAdvectedVenkatakrishnanDeferredCorrection", "adv_venkat_method_upwind", params);

    const auto contrib = method.advectedInterpolate(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);

    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
    EXPECT_DOUBLE_EQ(contrib.rhs_face_value, 0.0);
    EXPECT_DOUBLE_EQ(method.advectedInterpolateValue(
                         face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux),
                     elem_value);
  }

  {
    InputParameters params = _factory.getValidParams("FVAdvectedVenkatakrishnanDeferredCorrection");
    params.set<Real>("deferred_correction_factor") = 1.0;
    auto & method = addObject<FVAdvectedVenkatakrishnanDeferredCorrection>(
        "FVAdvectedVenkatakrishnanDeferredCorrection", "adv_venkat_method_full", params);

    const auto contrib = method.advectedInterpolate(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);

    const Point face_on_cn_line = face.faceCentroid() - face.skewnessCorrectionVector();
    const Point face_delta = face_on_cn_line - face.elemCentroid();
    const Real expected_phi_high = elem_value + (elem_grad * face_delta);

    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
    EXPECT_NEAR(contrib.rhs_face_value, elem_value - expected_phi_high, 1e-12);
    EXPECT_NEAR(method.advectedInterpolateValue(
                    face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux),
                expected_phi_high,
                1e-12);
  }
}
