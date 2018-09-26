//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MomentumFreeBC.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", MomentumFreeBC);

template <>
InputParameters
validParams<MomentumFreeBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("vel_x", "x-component of velocity");
  params.addCoupledVar("vel_y", "y-component of velocity");
  params.addCoupledVar("vel_z", "z-component of velocity");
  params.addRequiredParam<unsigned int>("component", "Component of the momentum equation");
  params.addRequiredCoupledVar("pressure", "The value of pressure");

  return params;
}

MomentumFreeBC::MomentumFreeBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _pressure(coupledValue("pressure")),
    _vel_x(coupledValue("vel_x")),
    _vel_y(_mesh.dimension() >= 2 ? coupledValue("vel_y") : _zero),
    _vel_z(_mesh.dimension() >= 3 ? coupledValue("vel_z") : _zero)
{
}

Real
MomentumFreeBC::computeQpResidual()
{
  RealVectorValue vel_vec(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]);
  return (_u[_qp] * vel_vec * _normals[_qp] + _pressure[_qp] * _normals[_qp](_component)) *
         _test[_i][_qp];
}
