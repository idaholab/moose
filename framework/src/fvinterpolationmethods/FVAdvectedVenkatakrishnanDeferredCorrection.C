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
  const DeviceData data{_deferred_correction_factor};
  setAdvectedSystemContributionCalculator(buildAdvectedSystemContributionCalculatorLimited<
                                          FVAdvectedVenkatakrishnanDeferredCorrection>(data));
  setAdvectedFaceValueInterpolator(
      buildAdvectedFaceValueInterpolatorLimited<FVAdvectedVenkatakrishnanDeferredCorrection>(data));
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedVenkatakrishnanDeferredCorrection::advectedInterpolate(
    const FaceInfo & face,
    const Real elem_value,
    const Real neighbor_value,
    const VectorValue<Real> * const elem_grad,
    const VectorValue<Real> * const neighbor_grad,
    const Real mass_flux) const
{
  return advectedInterpolate(DeviceData{_deferred_correction_factor},
                             face,
                             elem_value,
                             neighbor_value,
                             elem_grad,
                             neighbor_grad,
                             mass_flux);
}

Real
FVAdvectedVenkatakrishnanDeferredCorrection::advectedInterpolateValue(
    const FaceInfo & face,
    const Real elem_value,
    const Real neighbor_value,
    const VectorValue<Real> * const elem_grad,
    const VectorValue<Real> * const neighbor_grad,
    const Real mass_flux) const
{
  return advectedInterpolateValue(DeviceData{_deferred_correction_factor},
                                  face,
                                  elem_value,
                                  neighbor_value,
                                  elem_grad,
                                  neighbor_grad,
                                  mass_flux);
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedVenkatakrishnanDeferredCorrection::advectedInterpolate(
    const DeviceData & data,
    const FaceInfo & face,
    const Real elem_value,
    const Real neighbor_value,
    const VectorValue<Real> * const elem_grad,
    const VectorValue<Real> * const neighbor_grad,
    const Real mass_flux)
{
  mooseAssert(elem_grad && neighbor_grad,
              "Venkatakrishnan deferred correction requires both element and neighbor gradients.");

  // Use a branchless selection for upwind/downwind quantities.
  const Real upwind_mask = mass_flux >= 0.0;
  const Real downwind_mask = 1.0 - upwind_mask;

  const Real phi_upwind = upwind_mask * elem_value + downwind_mask * neighbor_value;

  const VectorValue<Real> grad_upwind =
      upwind_mask * (*elem_grad) + downwind_mask * (*neighbor_grad);

  // Reconstruct a higher-order face value from the upwind cell using the (limited) cell gradient.
  // We use a skewness-corrected face centroid projected to the line connecting cell centroids.
  const Point face_on_cn_line = face.faceCentroid() - face.skewnessCorrectionVector();
  const Point upwind_centroid =
      upwind_mask * face.elemCentroid() + downwind_mask * face.neighborCentroid();
  const Point face_delta = face_on_cn_line - upwind_centroid;

  const Real phi_high = phi_upwind + (grad_upwind * face_delta);

  AdvectedSystemContribution result;
  // Matrix contribution: pure upwind.
  result.weights_matrix = std::make_pair(upwind_mask, downwind_mask);

  const Real phi_matrix =
      result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
  // Deferred correction: add the difference between low-order and high-order reconstructions
  // explicitly on the RHS (scaled for robustness).
  result.rhs_face_value = data.deferred_correction_factor * (phi_matrix - phi_high);

  return result;
}

Real
FVAdvectedVenkatakrishnanDeferredCorrection::advectedInterpolateValue(
    const DeviceData & data,
    const FaceInfo & face,
    const Real elem_value,
    const Real neighbor_value,
    const VectorValue<Real> * const elem_grad,
    const VectorValue<Real> * const neighbor_grad,
    const Real mass_flux)
{
  const auto result =
      advectedInterpolate(data, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
  const Real phi_matrix =
      result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
  return phi_matrix - result.rhs_face_value;
}
