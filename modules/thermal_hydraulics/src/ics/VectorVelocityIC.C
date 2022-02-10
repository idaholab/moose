//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorVelocityIC.h"
#include "Function.h"
#include "FEProblemBase.h"

registerMooseObject("ThermalHydraulicsApp", VectorVelocityIC);

InputParameters
VectorVelocityIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<FunctionName>("vel_fn", "Velocity function name");
  params.addRequiredParam<unsigned int>("component", "The vector component of interest");
  params.addClassDescription(
      "Computes velocity in the direction of a 1-D element from a vector velocity function");
  return params;
}

VectorVelocityIC::VectorVelocityIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _vel_fn(getFunction("vel_fn")),
    _component(getParam<unsigned int>("component"))
{
}

Real
VectorVelocityIC::value(const Point & p)
{
  const Elem * el = _fe_problem.mesh().elemPtr(_current_elem->id());
  RealVectorValue dir = el->node_ref(1) - el->node_ref(0);
  return _vel_fn.value(_t, p) * dir(_component) / dir.norm();
}
