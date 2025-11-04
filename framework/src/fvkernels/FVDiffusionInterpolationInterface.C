//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDiffusionInterpolationInterface.h"

InputParameters
FVDiffusionInterpolationInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum face_interp_method("average skewness-corrected", "average");
  params.addParam<MooseEnum>(
      "variable_interp_method",
      face_interp_method,
      "Switch that can select between face interpolation methods for the variable.");

  return params;
}

FVDiffusionInterpolationInterface::FVDiffusionInterpolationInterface(const InputParameters & params)
  : _var_interp_method(
        Moose::FV::selectInterpolationMethod(params.get<MooseEnum>("variable_interp_method"))),
    _correct_skewness(_var_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage)
{
}
