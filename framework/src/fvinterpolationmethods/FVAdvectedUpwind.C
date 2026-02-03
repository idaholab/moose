//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVAdvectedUpwind.h"

registerMooseObject("MooseApp", FVAdvectedUpwind);

InputParameters
FVAdvectedUpwind::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription(
      "Upwind interpolation for advected quantities using the face mass flux sign.");
  return params;
}

FVAdvectedUpwind::FVAdvectedUpwind(const InputParameters & params) : FVInterpolationMethod(params)
{
  const DeviceData data{};
  setAdvectedSystemContributionCalculator(
      buildAdvectedSystemContributionCalculator<FVAdvectedUpwind>(data, false));
  setAdvectedFaceValueInterpolator(
      buildAdvectedFaceValueInterpolator<FVAdvectedUpwind>(data, false));
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedUpwind::advectedInterpolate(const FaceInfo & face,
                                      Real elem_value,
                                      Real neighbor_value,
                                      const VectorValue<Real> * elem_grad,
                                      const VectorValue<Real> * neighbor_grad,
                                      Real mass_flux) const
{
  return advectedInterpolate(
      DeviceData{}, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}

FVInterpolationMethod::AdvectedSystemContribution
FVAdvectedUpwind::advectedInterpolate(const DeviceData &,
                                      const FaceInfo & /*face*/,
                                      Real /*elem_value*/,
                                      Real /*neighbor_value*/,
                                      const VectorValue<Real> * /*elem_grad*/,
                                      const VectorValue<Real> * /*neighbor_grad*/,
                                      Real mass_flux)
{
  AdvectedSystemContribution result;
  // Branchless upwind selection to keep interpolation SIMD/GPU friendly
  const Real neighbor_weight = mass_flux < 0.0;
  result.weights_matrix = std::make_pair(1.0 - neighbor_weight, neighbor_weight);
  return result;
}

Real
FVAdvectedUpwind::advectedInterpolateValue(const FaceInfo & face,
                                           Real elem_value,
                                           Real neighbor_value,
                                           const VectorValue<Real> * elem_grad,
                                           const VectorValue<Real> * neighbor_grad,
                                           Real mass_flux) const
{
  const auto result = advectedInterpolate(
      DeviceData{}, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
  return result.weights_matrix.first * elem_value + result.weights_matrix.second * neighbor_value;
}

Real
FVAdvectedUpwind::advectedInterpolateValue(const DeviceData & data,
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
