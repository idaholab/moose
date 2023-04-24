//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumPositions.h"
#include "Positions.h"

registerMooseObject("MooseApp", NumPositions);

InputParameters
NumPositions::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<PositionsName>("positions", "Positions to count the number of");

  params.addClassDescription("Return the number of Positions from a Positions object.");
  return params;
}

NumPositions::NumPositions(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _positions(_fe_problem.getPositionsObject(getParam<PositionsName>("positions")))
{
}

Real
NumPositions::getValue()
{
  if (_fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL)
    return _positions.getPositions(/*initial=*/true).size();
  else
    return _positions.getPositions(/*initial=*/false).size();
}
