//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostAux.h"
#include "GhostUserObject.h"

template <>
InputParameters
validParams<GhostAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<UserObjectName>("ghost_user_object",
                                  "The GhostUserObject where this Aux pulls values from");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  params.addClassDescription("Aux Kernel to display ghosted elements from a single processor or "
                             "the union on all processors");
  return params;
}

GhostAux::GhostAux(const InputParameters & params)
  : AuxKernel(params), _ghost_uo(getUserObject<GhostUserObject>("ghost_user_object"))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
GhostAux::computeValue()
{
  return _ghost_uo.getElementalValue(_current_elem->id());
}
