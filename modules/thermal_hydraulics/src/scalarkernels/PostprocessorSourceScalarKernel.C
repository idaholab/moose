//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorSourceScalarKernel.h"

registerMooseObject("ThermalHydraulicsApp", PostprocessorSourceScalarKernel);

InputParameters
PostprocessorSourceScalarKernel::validParams()
{
  InputParameters params = ODEKernel::validParams();

  params.addRequiredParam<PostprocessorName>("pp", "Post-processor to act as source");

  params.addClassDescription("Adds arbitrary post-processor value as source term");

  return params;
}

PostprocessorSourceScalarKernel::PostprocessorSourceScalarKernel(const InputParameters & params)
  : ODEKernel(params),

    _pp(getPostprocessorValue("pp"))
{
}

Real
PostprocessorSourceScalarKernel::computeQpResidual()
{
  return -_pp;
}
