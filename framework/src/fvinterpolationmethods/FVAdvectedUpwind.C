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
  setAdvectedFaceInterpolator(buildAdvectedFaceInterpolator<FVAdvectedUpwind>(false));
}

FVInterpolationMethod::AdvectedInterpolationResult
FVAdvectedUpwind::advectedInterpolate(const FaceInfo & /*face*/,
                                      Real /*elem_value*/,
                                      Real /*neighbor_value*/,
                                      const VectorValue<Real> * /*elem_grad*/,
                                      const VectorValue<Real> * /*neighbor_grad*/,
                                      Real mass_flux) const
{
  AdvectedInterpolationResult result;
  if (mass_flux >= 0.0)
    result.weights_matrix = result.weights_high = std::make_pair(1.0, 0.0);
  else
    result.weights_matrix = result.weights_high = std::make_pair(0.0, 1.0);

  result.has_correction = false;
  return result;
}
