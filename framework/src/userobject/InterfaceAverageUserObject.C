//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceAverageUserObject.h"
#include "InterfaceAverageTools.h"

template <>
InputParameters
validParams<InterfaceAverageUserObject>()
{
  InputParameters params = validParams<InterfaceUserObject>();
  params.addParam<MooseEnum>(
      "average_type", InterfaceAverageTools::InterfaceAverageOptions(), "Type of scalar output");
  return params;
}

InterfaceAverageUserObject::InterfaceAverageUserObject(const InputParameters & parameters)
  : InterfaceUserObject(parameters), _average_type(parameters.get<MooseEnum>("average_type"))
{
}

Real
InterfaceAverageUserObject::ComputeAverageType(const Real value_master, const Real value_slave)
{
  return InterfaceAverageTools::getQuantity(_average_type, value_master, value_slave);
}
