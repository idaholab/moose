//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterpolationMethod.h"

InputParameters
FVInterpolationMethod::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("FVInterpolationMethod");
  params.registerSystemAttributeName("FVInterpolationMethod");
  params.addClassDescription(
      "Base class for defining face interpolation schemes used by finite volume objects.");
  return params;
}

FVInterpolationMethod::FVInterpolationMethod(const InputParameters & params)
  : MooseObject(params)
{
  // Default: no advected interpolator provided by base class
}
