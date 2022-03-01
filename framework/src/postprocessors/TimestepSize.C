//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimestepSize.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", TimestepSize);

InputParameters
TimestepSize::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Reports the timestep size");
  return params;
}

TimestepSize::TimestepSize(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _feproblem(dynamic_cast<FEProblemBase &>(_subproblem))
{
}

Real
TimestepSize::getValue()
{
  return _feproblem.dt();
}
