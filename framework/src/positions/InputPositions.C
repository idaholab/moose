//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputPositions.h"

registerMooseObject("MooseApp", InputPositions);

InputParameters
InputPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription("Positions set directly from a user parameter in the input file");
  params.addRequiredParam<std::vector<Point>>("positions", "Positions");
  return params;
}

InputPositions::InputPositions(const InputParameters & parameters) : Positions(parameters)
{
  _positions = getParam<std::vector<Point>>("positions");
}
