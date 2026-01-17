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
  auto & method =
      addObject<FVGeometricAverage>("FVGeometricAverage", "geom_interp_method", params);

  const Real elem_value = 2.0;
  const Real neighbor_value = 10.0;
  const Real expected =
      _internal_face->gC() * elem_value + (1.0 - _internal_face->gC()) * neighbor_value;

  EXPECT_NEAR(method.faceInterpolator()(*_internal_face, elem_value, neighbor_value), expected, 1e-12);
}

TEST_F(FVInterpolationMethodTest, harmonicAverage)
{
  InputParameters params = _factory.getValidParams("FVHarmonicAverage");
  auto & method =
      addObject<FVHarmonicAverage>("FVHarmonicAverage", "harm_interp_method", params);

  const Real elem_value = 4.0;
  const Real neighbor_value = 1.0;

  const Real gc = _internal_face->gC();
  const Real expected = 1.0 / (gc / elem_value + (1.0 - gc) / neighbor_value);

  EXPECT_NEAR(method.faceInterpolator()(*_internal_face, elem_value, neighbor_value), expected, 1e-12);
}

TEST_F(FVInterpolationMethodTest, advectedUpwind)
{
  InputParameters params = _factory.getValidParams("FVAdvectedUpwind");
  auto & method = addObject<FVAdvectedUpwind>("FVAdvectedUpwind", "adv_upwind_method", params);

  const Real elem_value = 2.0;
  const Real neighbor_value = 10.0;

  {
    const Real mass_flux = 1.0;
    const auto contrib = method.advectedSystemContributionCalculator()(
        *_internal_face, elem_value, neighbor_value, nullptr, nullptr, mass_flux);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
    EXPECT_DOUBLE_EQ(contrib.rhs_face_value, 0.0);
    EXPECT_DOUBLE_EQ(method.advectedFaceValueInterpolator()(
                         *_internal_face, elem_value, neighbor_value, nullptr, nullptr, mass_flux),
                     elem_value);
  }

  {
    const Real mass_flux = -1.0;
    const auto contrib = method.advectedSystemContributionCalculator()(
        *_internal_face, elem_value, neighbor_value, nullptr, nullptr, mass_flux);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 0.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 1.0);
    EXPECT_DOUBLE_EQ(contrib.rhs_face_value, 0.0);
    EXPECT_DOUBLE_EQ(method.advectedFaceValueInterpolator()(
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
    const auto contrib = method.advectedSystemContributionCalculator()(
        *_internal_face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
  }

  {
    const Real mass_flux = -1.0;
    const auto contrib = method.advectedSystemContributionCalculator()(
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
    const auto contrib = method.advectedSystemContributionCalculator()(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_NEAR(contrib.weights_matrix.first, expected_w_elem, 1e-12);
    EXPECT_NEAR(contrib.weights_matrix.second, expected_w_neighbor, 1e-12);
  }

  {
    const Real mass_flux = -1.0;
    const auto contrib = method.advectedSystemContributionCalculator()(
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

  const Real elem_value = 3.0;
  const Real neighbor_value = 3.0;

  const VectorValue<Real> elem_grad = face.dCN();
  const VectorValue<Real> neighbor_grad = -face.dCN();

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

  const Real r_f = 1e6;
  const Real beta = (r_f + r_f) / (1.0 + r_f);

  auto expected_unclamped = [&](const Real mass_flux)
  {
    const bool elem_is_upwind = mass_flux >= 0.0;
    const Real w_f = elem_is_upwind ? gc : (1.0 - gc);
    const Real g = beta * (1.0 - w_f);

    const Real w_upwind = 1.0 - g;
    const Real w_downwind = g;
    const Real w_elem = elem_is_upwind ? w_upwind : w_downwind;
    const Real w_neighbor = elem_is_upwind ? w_downwind : w_upwind;
    return std::make_pair(w_elem, w_neighbor);
  };

  for (const Real mass_flux : {1.0, -1.0})
  {
    const auto contrib_clamped = method_clamped.advectedSystemContributionCalculator()(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    EXPECT_NEAR(contrib_clamped.weights_matrix.first, expected_w_elem_linear, 1e-12);
    EXPECT_NEAR(contrib_clamped.weights_matrix.second, expected_w_neighbor_linear, 1e-12);

    const auto contrib_unclamped = method_unclamped.advectedSystemContributionCalculator()(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
    const auto expected = expected_unclamped(mass_flux);
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

    const auto contrib = method.advectedSystemContributionCalculator()(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);

    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
    EXPECT_DOUBLE_EQ(contrib.rhs_face_value, 0.0);
    EXPECT_DOUBLE_EQ(method.advectedFaceValueInterpolator()(
                         face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux),
                     elem_value);
  }

  {
    InputParameters params = _factory.getValidParams("FVAdvectedVenkatakrishnanDeferredCorrection");
    params.set<Real>("deferred_correction_factor") = 1.0;
    auto & method = addObject<FVAdvectedVenkatakrishnanDeferredCorrection>(
        "FVAdvectedVenkatakrishnanDeferredCorrection", "adv_venkat_method_full", params);

    const auto contrib = method.advectedSystemContributionCalculator()(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);

    const Point face_on_cn_line = face.faceCentroid() - face.skewnessCorrectionVector();
    const Point face_delta = face_on_cn_line - face.elemCentroid();
    const Real expected_phi_high = elem_value + (elem_grad * face_delta);

    EXPECT_DOUBLE_EQ(contrib.weights_matrix.first, 1.0);
    EXPECT_DOUBLE_EQ(contrib.weights_matrix.second, 0.0);
    EXPECT_NEAR(contrib.rhs_face_value, elem_value - expected_phi_high, 1e-12);
    EXPECT_NEAR(method.advectedFaceValueInterpolator()(
                    face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux),
                expected_phi_high,
                1e-12);
  }
}
