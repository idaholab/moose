//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowNearestQp.h"

template <>
InputParameters
validParams<PorousFlowNearestQp>()
{
  InputParameters params = validParams<PorousFlowMaterial>();
  params.set<bool>("nodal_material") = true;
  params.addClassDescription("Provides the nearest quadpoint to a node in each element");
  return params;
}

PorousFlowNearestQp::PorousFlowNearestQp(const InputParameters & parameters)
  : PorousFlowMaterial(parameters),
    _nearest_qp(declareProperty<unsigned>("PorousFlow_nearestqp_nodal"))
{
  if (getParam<bool>("nodal_material") == false)
    mooseError("PorousFlowNearestQp must be a nodal material");
}

void
PorousFlowNearestQp::computeQpProperties()
{
  _nearest_qp[_qp] = nearestQP(_qp);
}
