//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumCouplingIterations.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", NumCouplingIterations);

InputParameters
NumCouplingIterations::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Returns the number of multiapp coupling iterations taken by the executioner.");
  return params;
}

NumCouplingIterations::NumCouplingIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
}

Real
NumCouplingIterations::getValue()
{
  return _app.getExecutioner()->iterativeMultiAppSolve()->numCouplingIts();
}
