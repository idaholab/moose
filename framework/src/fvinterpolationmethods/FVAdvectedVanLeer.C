//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVAdvectedVanLeer.h"

#include <limits>

registerMooseObject("MooseApp", FVAdvectedVanLeer);

InputParameters
FVAdvectedVanLeer::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription(
      "TVD Van Leer interpolation for advected quantities using cell gradients (deferred "
      "correction only).");
  return params;
}

FVAdvectedVanLeer::FVAdvectedVanLeer(const InputParameters & params)
  : FVInterpolationMethod(params),
    _limiter(Moose::FV::Limiter<Real>::build(Moose::FV::LimiterType::VanLeer))
{
  setAdvectedSystemContributionCalculator(
      buildAdvectedSystemContributionCalculator<FVAdvectedVanLeer>(true));
  setAdvectedFaceValueInterpolator(buildAdvectedFaceValueInterpolator<FVAdvectedVanLeer>(true));
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedVanLeer::advectedInterpolate(const FaceInfo & face,
                                       Real elem_value,
                                       Real neighbor_value,
                                       const VectorValue<Real> * elem_grad,
                                       const VectorValue<Real> * neighbor_grad,
                                       Real mass_flux) const
{
  mooseAssert(elem_grad && neighbor_grad,
              "Van Leer advected interpolation requires both element and neighbor gradients.");

  const bool elem_is_upwind = mass_flux >= 0.0;
  const Real upwind_mask = elem_is_upwind;          // 1.0 when elem upwind, 0.0 otherwise
  const Real downwind_mask = 1.0 - upwind_mask;     // 1.0 when neighbor upwind

  const Real phi_high = advectedInterpolateValue(face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);

  AdvectedSystemContribution result;
  // Pure upwind matrix contribution (deferred correction carries the high-order part)
  result.weights_matrix = std::make_pair(upwind_mask, downwind_mask);
  // RHS carries only the deferred correction flux contribution
  const Real phi_matrix = result.weights_matrix.first * elem_value +
                          result.weights_matrix.second * neighbor_value;
  result.rhs_face_value = phi_high - phi_matrix;

  return result;
}

Real
FVAdvectedVanLeer::advectedInterpolateValue(const FaceInfo & face,
                                            Real elem_value,
                                            Real neighbor_value,
                                            const VectorValue<Real> * elem_grad,
                                            const VectorValue<Real> * neighbor_grad,
                                            Real mass_flux) const
{
  mooseAssert(elem_grad && neighbor_grad,
              "Van Leer advected interpolation requires both element and neighbor gradients.");

  const bool elem_is_upwind = mass_flux >= 0.0;
  const Real upwind_mask = elem_is_upwind;
  const Real downwind_mask = 1.0 - upwind_mask;

  const Real phi_upwind = upwind_mask * elem_value + downwind_mask * neighbor_value;
  const Real phi_downwind = upwind_mask * neighbor_value + downwind_mask * elem_value;

  const VectorValue<Real> grad_upwind =
      upwind_mask * (*elem_grad) + downwind_mask * (*neighbor_grad);

  const auto beta = _limiter->limit(phi_upwind,
                                    phi_downwind,
                                    &grad_upwind,
                                    /*grad_phi_downwind*/ nullptr,
                                    face.dCN() * (upwind_mask * 2.0 - 1.0),
                                    /*max_value*/ std::numeric_limits<Real>::max(),
                                    /*min_value*/ std::numeric_limits<Real>::lowest(),
                                    &face,
                                    elem_is_upwind);

  const Point face_delta =
      face.faceCentroid() - (elem_is_upwind ? face.elemCentroid() : face.neighborCentroid());
  return phi_upwind + beta * (grad_upwind * face_delta);
}
