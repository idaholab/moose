/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSStagnationBC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<NSStagnationBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D
  params.addRequiredCoupledVar("temperature", "");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");
  params.addRequiredParam<Real>("R", "Gas constant.");
  return params;
}

NSStagnationBC::NSStagnationBC(const InputParameters & parameters) :
    NodalBC(parameters),
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _temperature(coupledValue("temperature")),
    _gamma(getParam<Real>("gamma")),
    _R(getParam<Real>("R"))
{
}
