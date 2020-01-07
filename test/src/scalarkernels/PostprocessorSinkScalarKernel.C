//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorSinkScalarKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"

registerMooseObject("MooseTestApp", PostprocessorSinkScalarKernel);

InputParameters
PostprocessorSinkScalarKernel::validParams()
{
  InputParameters params = ODEKernel::validParams();
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "Name of the Postprocessor whose value will be the sink");

  return params;
}

PostprocessorSinkScalarKernel::PostprocessorSinkScalarKernel(const InputParameters & parameters)
  : ODEKernel(parameters), _pp_value(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorSinkScalarKernel::computeQpResidual()
{
  return _pp_value;
}
