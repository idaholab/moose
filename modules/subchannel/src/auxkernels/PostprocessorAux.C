//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorAux.h"
registerMooseObject("SubChannelApp", PostprocessorAux);

InputParameters
PostprocessorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Creates a constant field in the domain. Reads value from postprocessor");
  params.addRequiredParam<PostprocessorName>("postprocessor",
                                             "The postprocessor to use for the value");
  return params;
}

PostprocessorAux::PostprocessorAux(const InputParameters & parameters)
  : AuxKernel(parameters), _pvalue(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorAux::computeValue()
{
  return _pvalue;
}
