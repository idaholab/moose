//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVMomentumHLLC.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVMomentumHLLC);

InputParameters
CNSFVMomentumHLLC::validParams()
{
  InputParameters params = CNSFVHLLC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addClassDescription(
      "Implements the momentum flux portion of the free-flow HLLC discretization.");
  return params;
}

CNSFVMomentumHLLC::CNSFVMomentumHLLC(const InputParameters & params)
  : CNSFVHLLC(params), _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVMomentumHLLC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp] * _vel_elem[_qp](_index) +
         _normal(_index) * _pressure_elem[_qp];
}

ADReal
CNSFVMomentumHLLC::fluxNeighbor()
{
  return _normal_speed_neighbor * _rho_neighbor[_qp] * _vel_neighbor[_qp](_index) +
         _normal(_index) * _pressure_neighbor[_qp];
}

ADReal
CNSFVMomentumHLLC::hllcElem()
{
  auto vel_nonnormal = _vel_elem[_qp] - _normal_speed_elem * _normal;
  return _normal(_index) * _SM + vel_nonnormal(_index);

  // For some reason, the below expression doesn't give as good results as the
  // above one.
  // return _normal(_index) * (_SM - _normal_speed_elem) + _vel_elem[_qp](_index);
}

ADReal
CNSFVMomentumHLLC::hllcNeighbor()
{
  auto vel_nonnormal = _vel_neighbor[_qp] - _normal_speed_neighbor * _normal;
  return _normal(_index) * _SM + vel_nonnormal(_index);

  // For some reason, the below expression doesn't give as good results as the
  // above one.
  // return _normal(_index) * (_SM - _normal_speed_neighbor) + _vel_elem[_qp](_index);
}

ADReal
CNSFVMomentumHLLC::conservedVariableElem()
{
  return _rho_elem[_qp] * _vel_elem[_qp](_index);
}

ADReal
CNSFVMomentumHLLC::conservedVariableNeighbor()
{
  return _rho_neighbor[_qp] * _vel_neighbor[_qp](_index);
}
