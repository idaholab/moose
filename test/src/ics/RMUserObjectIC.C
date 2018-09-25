//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RMUserObjectIC.h"
#include "AlgebraicRMTester.h"

registerMooseObject("MooseTestApp", RMUserObjectIC);

template <>
InputParameters
validParams<RMUserObjectIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("rm_user_object",
                                          "The RMUserObject to be coupled into this IC");
  return params;
}

RMUserObjectIC::RMUserObjectIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _rm_uo(getUserObject<AlgebraicRMTester>("rm_user_object"))
{
}

Real
RMUserObjectIC::value(const Point & /*p*/)
{
  if (_current_elem == NULL)
    return -1.0;

  return _rm_uo.getElementalValue(_current_elem->id());
}
