//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RMAux.h"
#include "AlgebraicRMTester.h"

registerMooseObject("MooseTestApp", RMAux);

template <>
InputParameters
validParams<RMAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<UserObjectName>("rm_user_object",
                                  "The RMUserObject where this Aux pulls values from");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  params.addClassDescription("Aux Kernel to display ghosted elements from a single processor or "
                             "the union on all processors");
  return params;
}

RMAux::RMAux(const InputParameters & params)
  : AuxKernel(params), _rm_uo(getUserObject<AlgebraicRMTester>("rm_user_object"))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
RMAux::computeValue()
{
  return _rm_uo.getElementalValue(_current_elem->id());
}
