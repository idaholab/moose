//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAdvectedVanLeerWeightBased.h"

#include "MathFVUtils.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", FVAdvectedVanLeerWeightBased);

InputParameters
FVAdvectedVanLeerWeightBased::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription(
      "Van Leer interpolation for advected quantities implemented as limited blending weights "
      "(no MUSCL reconstruction, no deferred correction).");
  params.addParam<bool>(
      "limit_to_linear",
      true,
      "Whether to limit the scheme to be no more downwind-biased than linear interpolation "
      "(blends between upwind and linear; avoids 'compressive' weights that can lead to "
      "downwind weighting and poor linear solver behavior).");
  params.addRangeCheckedParam<Real>(
      "blending_factor",
      1.0,
      "blending_factor>=0 & blending_factor<=1",
      "Scales the high-order blending strength; 0 gives pure upwind, 1 gives the full limited "
      "blending. Values < 1 can improve linear solver robustness for fully implicit assembly.");
  return params;
}

FVAdvectedVanLeerWeightBased::FVAdvectedVanLeerWeightBased(const InputParameters & params)
  : FVInterpolationMethod(params),
    _limit_to_linear(getParam<bool>("limit_to_linear")),
    _blending_factor(getParam<Real>("blending_factor"))
{
  const DeviceData data{_limit_to_linear, _blending_factor};
  setAdvectedSystemContributionCalculator(
      buildAdvectedSystemContributionCalculator<FVAdvectedVanLeerWeightBased>(data, true));
  setAdvectedFaceValueInterpolator(
      buildAdvectedFaceValueInterpolator<FVAdvectedVanLeerWeightBased>(data, true));
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedVanLeerWeightBased::advectedInterpolate(const FaceInfo & face,
                                                  Real elem_value,
                                                  Real neighbor_value,
                                                  const VectorValue<Real> * elem_grad,
                                                  const VectorValue<Real> * neighbor_grad,
                                                  Real mass_flux) const
{
  const DeviceData data{_limit_to_linear, _blending_factor};
  return advectedInterpolate(
      data, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}

Real
FVAdvectedVanLeerWeightBased::advectedInterpolateValue(const FaceInfo & face,
                                                       Real elem_value,
                                                       Real neighbor_value,
                                                       const VectorValue<Real> * elem_grad,
                                                       const VectorValue<Real> * neighbor_grad,
                                                       Real mass_flux) const
{
  const auto result = advectedInterpolate(DeviceData{_limit_to_linear, _blending_factor},
                                          face,
                                          elem_value,
                                          neighbor_value,
                                          elem_grad,
                                          neighbor_grad,
                                          mass_flux);
  return result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedVanLeerWeightBased::advectedInterpolate(const DeviceData & data,
                                                  const FaceInfo & face,
                                                  Real elem_value,
                                                  Real neighbor_value,
                                                  const VectorValue<Real> * elem_grad,
                                                  const VectorValue<Real> * neighbor_grad,
                                                  Real mass_flux)
{
  mooseAssert(elem_grad && neighbor_grad,
              "Van Leer advected interpolation requires both element and neighbor gradients.");

  // Use a branchless selection for upwind/downwind quantities (SIMD/GPU friendly when we get
  // there).
  const Real upwind_mask = mass_flux >= 0.0;
  const Real downwind_mask = 1.0 - upwind_mask;

  const Real phi_upwind = upwind_mask * elem_value + downwind_mask * neighbor_value;
  const Real phi_downwind = upwind_mask * neighbor_value + downwind_mask * elem_value;

  const VectorValue<Real> grad_upwind =
      upwind_mask * (*elem_grad) + downwind_mask * (*neighbor_grad);

  const auto r_f = Moose::FV::rFBranchless(
      phi_upwind, phi_downwind, grad_upwind, face.dCN() * (2.0 * upwind_mask - 1.0));
  using std::abs;
  const Real beta = (r_f + abs(r_f)) / (1.0 + abs(r_f));

  // Geometric weight associated with the upwind cell for this face.
  const Real w_f = upwind_mask * face.gC() + downwind_mask * (1.0 - face.gC());

  // Following the Greenshields blending form:
  //   phi_f = (1-g)*phi_upwind + g*phi_downwind, with g = beta*(1-w_f)
  const Real g_unclamped = data.blending_factor * beta * (1.0 - w_f);
  const Real g_clamped = std::min(std::max(g_unclamped, 0.0), 1.0 - w_f);
  const Real clamp_mask = static_cast<Real>(data.limit_to_linear);
  // Clamp to [0, 1-w_f] so the weights do not become more downwind-biased than linear.
  const Real g = clamp_mask * g_clamped + (1.0 - clamp_mask) * g_unclamped;

  const Real w_upwind = 1.0 - g;
  const Real w_downwind = g;

  // Map (upwind, downwind) weights back to (elem, neighbor) ordering without branches.
  const Real w_elem = upwind_mask * w_upwind + downwind_mask * w_downwind;
  const Real w_neighbor = upwind_mask * w_downwind + downwind_mask * w_upwind;

  AdvectedSystemContribution result;
  result.weights_matrix = std::make_pair(w_elem, w_neighbor);

  return result;
}

Real
FVAdvectedVanLeerWeightBased::advectedInterpolateValue(const DeviceData & data,
                                                       const FaceInfo & face,
                                                       Real elem_value,
                                                       Real neighbor_value,
                                                       const VectorValue<Real> * elem_grad,
                                                       const VectorValue<Real> * neighbor_grad,
                                                       Real mass_flux)
{
  const auto result = advectedInterpolate(
      data, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
  return result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
}
