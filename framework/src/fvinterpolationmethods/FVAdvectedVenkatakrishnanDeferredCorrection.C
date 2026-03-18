//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAdvectedVenkatakrishnanDeferredCorrection.h"

registerMooseObject("MooseApp", FVAdvectedVenkatakrishnanDeferredCorrection);

InputParameters
FVAdvectedVenkatakrishnanDeferredCorrection::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription(
      "MUSCL reconstruction with Venkatakrishnan-limited cell gradients using deferred "
      "correction.");
  params.addRangeCheckedParam<Real>(
      "deferred_correction_factor",
      1.0,
      "deferred_correction_factor>=0 & deferred_correction_factor<=1",
      "Scales the deferred correction strength; 0 gives pure upwind (no deferred correction), 1 "
      "gives full deferred correction. Values < 1 can improve fixed point robustness.");
  return params;
}

FVAdvectedVenkatakrishnanDeferredCorrection::FVAdvectedVenkatakrishnanDeferredCorrection(
    const InputParameters & params)
  : FVInterpolationMethod(params),
    _deferred_correction_factor(getParam<Real>("deferred_correction_factor"))
{
}

FVAdvectedInterpolationMethod::AdvectedSystemContribution
FVAdvectedVenkatakrishnanDeferredCorrection::advectedInterpolate(
    const FaceInfo & face,
    const Real elem_value,
    const Real neighbor_value,
    const VectorValue<Real> * const elem_grad,
    const VectorValue<Real> * const neighbor_grad,
    const Real mass_flux) const
{
  mooseAssert(elem_grad && neighbor_grad,
              "Venkatakrishnan deferred correction requires both element and neighbor gradients.");

  const bool upwind_is_elem = mass_flux >= 0.0;
  const Real phi_upwind = upwind_is_elem ? elem_value : neighbor_value;
  const VectorValue<Real> grad_upwind = upwind_is_elem ? *elem_grad : *neighbor_grad;

  // Reconstruct a higher-order face value from the upwind cell using the (limited) cell gradient.
  const Point upwind_centroid = upwind_is_elem ? face.elemCentroid() : face.neighborCentroid();
  // For the Venkatakrishnan MUSCL scheme we reconstruct to the actual face centroid.
  const Point face_delta = face.faceCentroid() - upwind_centroid;

  const Real phi_high = phi_upwind + (grad_upwind * face_delta);

  AdvectedSystemContribution result;
  // Matrix contribution: pure upwind.
  result.weights_matrix = upwind_is_elem ? std::make_pair(1.0, 0.0) : std::make_pair(0.0, 1.0);

  const Real phi_matrix =
      result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
  // Deferred correction: add the difference between low-order and high-order reconstructions
  // explicitly on the RHS (scaled for robustness).
  result.rhs_face_value = _deferred_correction_factor * (phi_matrix - phi_high);

  return result;
}
