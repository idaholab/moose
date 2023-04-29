//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectionAux.h"

registerMooseObjectRenamed("MooseApp", SelfAux, "01/30/2024 24:00", ProjectionAux);
registerMooseObject("MooseApp", ProjectionAux);

InputParameters
ProjectionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Returns the specified variable as an auxiliary variable with a projection of the source "
      "variable. If they are the same type, this amounts to a simple copy.");
  params.addCoupledVar("v",
                       "Optional variable to take the value of. If omitted the value of the "
                       "`variable` itself is returned.");

  // Technically possible to project from nodal to elemental and back
  params.set<bool>("_allow_nodal_to_elemental_coupling") = true;
  return params;
}

ProjectionAux::ProjectionAux(const InputParameters & parameters)
  : AuxKernel(parameters), _v(isCoupled("v") ? coupledValue("v") : _u)
{
}

Real
ProjectionAux::computeValue()
{
  return _v[_qp];
}
