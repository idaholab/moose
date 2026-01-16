//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAdvectedVanLeerWeightBased.h"

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
  params.addDeprecatedParam<bool>("bounded",
                                  true,
                                  "Deprecated alias for 'limit_to_linear'.",
                                  "The 'bounded' parameter has been renamed to 'limit_to_linear'.");
  return params;
}

FVAdvectedVanLeerWeightBased::FVAdvectedVanLeerWeightBased(const InputParameters & params)
  : FVInterpolationMethod(params),
    _limiter(Moose::FV::Limiter<Real>::build(Moose::FV::LimiterType::VanLeer)),
    _limit_to_linear(
        [this]()
        {
          return isParamSetByUser("bounded") ? getParam<bool>("bounded")
                                             : getParam<bool>("limit_to_linear");
        }()),
    _blending_factor(getParam<Real>("blending_factor"))
{
  setAdvectedSystemContributionCalculator(
      buildAdvectedSystemContributionCalculator<FVAdvectedVanLeerWeightBased>(true));
  setAdvectedFaceValueInterpolator(
      buildAdvectedFaceValueInterpolator<FVAdvectedVanLeerWeightBased>(true));
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

  const bool elem_is_upwind = mass_flux >= 0.0;

  const Real phi_upwind = elem_is_upwind ? elem_value : neighbor_value;
  const Real phi_downwind = elem_is_upwind ? neighbor_value : elem_value;
  const VectorValue<Real> & grad_upwind = elem_is_upwind ? *elem_grad : *neighbor_grad;
  const VectorValue<Real> & grad_downwind = elem_is_upwind ? *neighbor_grad : *elem_grad;

  const auto beta = _limiter->limit(phi_upwind,
                                    phi_downwind,
                                    &grad_upwind,
                                    &grad_downwind,
                                    face.dCN() * (elem_is_upwind ? 1.0 : -1.0),
                                    /*max_value*/ std::numeric_limits<Real>::max(),
                                    /*min_value*/ std::numeric_limits<Real>::lowest(),
                                    &face,
                                    elem_is_upwind);

  // Geometric weight associated with the upwind cell for this face.
  const Real w_f = elem_is_upwind ? face.gC() : (1.0 - face.gC());

  // Following the Greenshields blending form used elsewhere in MOOSE:
  //   phi_f = (1-g)*phi_upwind + g*phi_downwind, with g = beta*(1-w_f)
  Real g = _blending_factor * beta * (1.0 - w_f);
  if (_limit_to_linear)
    g = std::clamp(g, 0.0, 1.0 - w_f);

  const Real w_upwind = 1.0 - g;
  const Real w_downwind = g;

  AdvectedSystemContribution result;
  result.rhs_face_value = 0.0;

  // Map (upwind, downwind) weights back to (elem, neighbor).
  if (elem_is_upwind)
    result.weights_matrix = std::make_pair(w_upwind, w_downwind);
  else
    result.weights_matrix = std::make_pair(w_downwind, w_upwind);

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
