//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVAdvectedGeometricAverage.h"

registerMooseObject("MooseApp", FVAdvectedGeometricAverage);

InputParameters
FVAdvectedGeometricAverage::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription(
      "Geometric average interpolation for advected quantities using FaceInfo weighting.");
  return params;
}

FVAdvectedGeometricAverage::FVAdvectedGeometricAverage(const InputParameters & params)
  : FVInterpolationMethod(params)
{
  setAdvectedFaceInterpolator(buildAdvectedFaceInterpolator<FVAdvectedGeometricAverage>(false));
}

FVInterpolationMethod::AdvectedInterpolationResult
FVAdvectedGeometricAverage::advectedInterpolate(const FaceInfo & face,
                                                Real /*elem_value*/,
                                                Real /*neighbor_value*/,
                                                const VectorValue<Real> * /*elem_grad*/,
                                                const VectorValue<Real> * /*neighbor_grad*/,
                                                Real /*mass_flux*/) const
{
  const Real gc = face.gC();
  AdvectedInterpolationResult result;
  result.weights_matrix = result.weights_high = std::make_pair(gc, 1.0 - gc);
  result.has_correction = false;
  return result;
}
