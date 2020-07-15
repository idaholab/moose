//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimePostprocessor.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", TimePostprocessor);

InputParameters
TimePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Reports the current time");
  return params;
}

TimePostprocessor::TimePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _feproblem(dynamic_cast<FEProblemBase &>(_subproblem))
{
}

Real
TimePostprocessor::getValue()
{
  return _feproblem.time();
}
