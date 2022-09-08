//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ObjectiveGradientMinimize.h"

registerMooseObject("OptimizationApp", ObjectiveGradientMinimize);

InputParameters
ObjectiveGradientMinimize::validParams()
{
  InputParameters params = OptimizationReporter::validParams();
  params.addClassDescription("OptimizationReporter that holds optimization information and "
                             "computes gradient from adjoint data.");
  return params;
}

ObjectiveGradientMinimize::ObjectiveGradientMinimize(const InputParameters & parameters)
  : OptimizationReporter(parameters)
{
}
