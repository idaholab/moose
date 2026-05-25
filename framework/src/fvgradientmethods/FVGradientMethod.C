//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGradientMethod.h"

InputParameters
FVGradientMethod::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("FVGradientMethod");
  params.registerSystemAttributeName("FVGradientMethod");
  params.addClassDescription("Base class for defining cell-centered gradient methods used by "
                             "linear finite volume objects.");
  return params;
}

FVGradientMethod::FVGradientMethod(const InputParameters & params) : MooseObject(params) {}
