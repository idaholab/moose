//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorFunction.h"
#include "MooseTypes.h"

registerMooseObject("MooseTestApp", PostprocessorFunction);

InputParameters
PostprocessorFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<PostprocessorName>(
      "pp", "The name of the postprocessor you are trying to get.");
  return params;
}

PostprocessorFunction::PostprocessorFunction(const InputParameters & parameters)
  : Function(parameters), _pp(getPostprocessorValue("pp"))
{
}

Real
PostprocessorFunction::value(Real /*t*/, const Point & /*p*/) const
{
  return _pp;
}
