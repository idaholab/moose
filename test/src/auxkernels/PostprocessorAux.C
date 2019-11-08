//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorAux.h"

registerMooseObject("MooseTestApp", PostprocessorAux);

InputParameters
PostprocessorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<PostprocessorName>("pp", "The Postprocessor to use as the value");
  return params;
}

PostprocessorAux::PostprocessorAux(const InputParameters & parameters)
  : AuxKernel(parameters), _pp_val(getPostprocessorValue("pp"))
{
}

PostprocessorAux::~PostprocessorAux() {}

Real
PostprocessorAux::computeValue()
{
  return _pp_val;
}
