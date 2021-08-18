//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestNodeNumber.h"
#include "UserObjectInterface.h"

registerMooseObject("MooseApp", NearestNodeNumber);

InputParameters
NearestNodeNumber::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("nearest_node_number_uo",
                                          "The NearestNodeNumberUO that computes the nearest node");
  params.addClassDescription("Outputs the nearest node number to a point");
  return params;
}

NearestNodeNumber::NearestNodeNumber(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _nnn(getUserObject<NearestNodeNumberUO>("nearest_node_number_uo"))
{
}

Real
NearestNodeNumber::getValue()
{
  return _nnn.getClosestNodeId();
}
