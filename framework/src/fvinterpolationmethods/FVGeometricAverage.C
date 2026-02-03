//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGeometricAverage.h"

registerMooseObject("MooseApp", FVGeometricAverage);

InputParameters
FVGeometricAverage::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription("Linear interpolation that uses the geometric weighting on FaceInfo.");
  return params;
}

FVGeometricAverage::FVGeometricAverage(const InputParameters & params)
  : FVInterpolationMethod(params)
{
  const DeviceData data{};
  setAdvectedSystemContributionCalculator(
      buildAdvectedSystemContributionCalculator<FVGeometricAverage>(data, false));
  setAdvectedFaceValueInterpolator(
      buildAdvectedFaceValueInterpolator<FVGeometricAverage>(data, false));
  setFaceInterpolator(buildFaceInterpolator<FVGeometricAverage>(data));
}

Real
FVGeometricAverage::interpolate(const FaceInfo & face,
                                const Real elem_value,
                                const Real neighbor_value) const
{
  return interpolate(DeviceData{}, face, elem_value, neighbor_value);
}

FVInterpolationMethod::AdvectedSystemContribution
FVGeometricAverage::advectedInterpolate(const FaceInfo & face,
                                        Real elem_value,
                                        Real neighbor_value,
                                        const VectorValue<Real> * elem_grad,
                                        const VectorValue<Real> * neighbor_grad,
                                        Real mass_flux) const
{
  return advectedInterpolate(
      DeviceData{}, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}

Real
FVGeometricAverage::advectedInterpolateValue(const FaceInfo & face,
                                             Real elem_value,
                                             Real neighbor_value,
                                             const VectorValue<Real> * elem_grad,
                                             const VectorValue<Real> * neighbor_grad,
                                             Real mass_flux) const
{
  return advectedInterpolateValue(
      DeviceData{}, face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
}

Real
FVGeometricAverage::interpolate(const DeviceData &,
                                const FaceInfo & face,
                                const Real elem_value,
                                const Real neighbor_value)
{
  const Real gc = face.gC();
  return gc * elem_value + (1.0 - gc) * neighbor_value;
}

FVInterpolationMethod::AdvectedSystemContribution
FVGeometricAverage::advectedInterpolate(const DeviceData &,
                                        const FaceInfo & face,
                                        Real /*elem_value*/,
                                        Real /*neighbor_value*/,
                                        const VectorValue<Real> * /*elem_grad*/,
                                        const VectorValue<Real> * /*neighbor_grad*/,
                                        Real /*mass_flux*/)
{
  AdvectedSystemContribution result;
  const Real gc = face.gC();
  result.weights_matrix = std::make_pair(gc, 1.0 - gc);
  return result;
}

Real
FVGeometricAverage::advectedInterpolateValue(const DeviceData & data,
                                             const FaceInfo & face,
                                             Real elem_value,
                                             Real neighbor_value,
                                             const VectorValue<Real> *,
                                             const VectorValue<Real> *,
                                             Real)
{
  return interpolate(data, face, elem_value, neighbor_value);
}
