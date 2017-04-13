/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
