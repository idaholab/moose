//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostUserObjectIC.h"
#include "GhostUserObject.h"

template <>
InputParameters
validParams<GhostUserObjectIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ghost_uo",
                                          "The GhostUserObject to be coupled into this IC");
  return params;
}

GhostUserObjectIC::GhostUserObjectIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _ghost_uo(getUserObject<GhostUserObject>("ghost_uo"))
{
}

Real
GhostUserObjectIC::value(const Point & /*p*/)
{
  if (_current_elem == NULL)
    return -1.0;

  return _ghost_uo.getElementalValue(_current_elem->id());
}
