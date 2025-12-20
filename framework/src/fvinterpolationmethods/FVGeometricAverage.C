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
  setFaceInterpolator(buildFaceInterpolator<FVGeometricAverage>());
}

Real
FVGeometricAverage::interpolate(const FaceInfo & face,
                                const Real elem_value,
                                const Real neighbor_value) const
{
  const Real gc = face.gC();
  return gc * elem_value + (1.0 - gc) * neighbor_value;
}
