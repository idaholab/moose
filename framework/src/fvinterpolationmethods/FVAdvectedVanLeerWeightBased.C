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
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedVanLeerWeightBased::advectedInterpolate(const FaceInfo & face,
                                                  Real elem_value,
                                                  Real neighbor_value,
                                                  const VectorValue<Real> * elem_grad,
                                                  const VectorValue<Real> * neighbor_grad,
                                                  Real mass_flux) const
{
  mooseAssert(elem_grad && neighbor_grad,
              "Van Leer advected interpolation requires both element and neighbor gradients.");

  const bool upwind_is_elem = mass_flux >= 0.0;

  const Real phi_upwind = upwind_is_elem ? elem_value : neighbor_value;
  const Real phi_downwind = upwind_is_elem ? neighbor_value : elem_value;

  const VectorValue<Real> grad_upwind = upwind_is_elem ? *elem_grad : *neighbor_grad;
  const Point upwind_to_downwind = upwind_is_elem ? face.dCN() : Point(-face.dCN());

  const auto r_f = Moose::FV::rF(phi_upwind, phi_downwind, grad_upwind, upwind_to_downwind);
  const Real beta = (r_f + std::abs(r_f)) / (1.0 + std::abs(r_f));

  // Geometric weight associated with the upwind cell for this face.
  const Real w_f = upwind_is_elem ? face.gC() : (1.0 - face.gC());

  // Following the Greenshields blending form:
  //   phi_f = (1-g)*phi_upwind + g*phi_downwind, with g = beta*(1-w_f)
  const Real g_unclamped = _blending_factor * beta * (1.0 - w_f);
  const Real g_clamped = std::min(std::max(g_unclamped, 0.0), 1.0 - w_f);
  // Clamp to [0, 1-w_f] so the weights do not become more downwind-biased than linear.
  const Real g = _limit_to_linear ? g_clamped : g_unclamped;

  const Real w_upwind = 1.0 - g;
  const Real w_downwind = g;

  // Map (upwind, downwind) weights back to (elem, neighbor) ordering.
  const Real w_elem = upwind_is_elem ? w_upwind : w_downwind;
  const Real w_neighbor = upwind_is_elem ? w_downwind : w_upwind;

  AdvectedSystemContribution result;
  result.weights_matrix = std::make_pair(w_elem, w_neighbor);

  return result;
}

Real
FVAdvectedVanLeerWeightBased::advectedInterpolateValue(const FaceInfo & face,
                                                       Real elem_value,
                                                       Real neighbor_value,
                                                       const VectorValue<Real> * elem_grad,
                                                       const VectorValue<Real> * neighbor_grad,
                                                       Real mass_flux) const
{
  const auto result =
      advectedInterpolate(face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
  return result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
}
