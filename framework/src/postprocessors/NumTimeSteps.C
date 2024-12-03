//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumTimeSteps.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", NumTimeSteps);

InputParameters
NumTimeSteps::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Reports the timestep number");
  return params;
}

NumTimeSteps::NumTimeSteps(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _feproblem(dynamic_cast<FEProblemBase &>(_subproblem))
{
}

Real
NumTimeSteps::getValue() const
{
  return _feproblem.timeStep();
}
