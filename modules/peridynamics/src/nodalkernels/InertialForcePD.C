//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InertialForcePD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", InertialForcePD);

template <>
InputParameters
validParams<InertialForcePD>()
{
  InputParameters params = validParams<NodalKernel>();
  params.addClassDescription("Class for calculating the residual for the interial force (M*accel)");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("density", "Value of material density");
  params.addRequiredCoupledVar("velocity", "Velocity variable");
  params.addRequiredCoupledVar("acceleration", "Acceleration variable");
  params.addRequiredParam<Real>("beta", "Beta parameter for Newmark Time integration");

  return params;
}

InertialForcePD::InertialForcePD(const InputParameters & parameters)
  : NodalKernel(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _density(getParam<Real>("density")),
    _u_old(valueOld()),
    _vel_old(coupledValueOld("velocity")),
    _accel_old(coupledValueOld("acceleration")),
    _beta(getParam<Real>("beta"))
{
}

Real
InertialForcePD::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else
  {
    Real accel = 1. / _beta *
                 (((_u[_qp] - _u_old[_qp]) / (_dt * _dt)) - _vel_old[_qp] / _dt -
                  _accel_old[_qp] * (0.5 - _beta));

    return _pdmesh.getVolume(_current_node->id()) * _density * accel;
  }
}

Real
InertialForcePD::computeQpJacobian()
{
  if (_dt == 0)
    return 0;
  else
    return _pdmesh.getVolume(_current_node->id()) * _density / (_beta * _dt * _dt);
}
