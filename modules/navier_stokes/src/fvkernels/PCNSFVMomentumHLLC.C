//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVMomentumHLLC.h"

registerMooseObject("NavierStokesApp", PCNSFVMomentumHLLC);

InputParameters
PCNSFVMomentumHLLC::validParams()
{
  InputParameters params = PCNSFVHLLC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addClassDescription(
      "Implements the momentum flux portion of the porous HLLC discretization.");
  return params;
}

PCNSFVMomentumHLLC::PCNSFVMomentumHLLC(const InputParameters & params)
  : PCNSFVHLLC(params), _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
PCNSFVMomentumHLLC::fluxElem()
{
  return _normal_speed_elem * _eps_elem[_qp] * _rho_elem[_qp] * _vel_elem[_qp](_index) +
         _normal(_index) * _eps_elem[_qp] * _pressure_elem[_qp];
}

ADReal
PCNSFVMomentumHLLC::fluxNeighbor()
{
  return _normal_speed_neighbor * _eps_neighbor[_qp] * _rho_neighbor[_qp] *
             _vel_neighbor[_qp](_index) +
         _normal(_index) * _eps_neighbor[_qp] * _pressure_neighbor[_qp];
}

ADReal
PCNSFVMomentumHLLC::hllcElem()
{
  auto vel_nonnormal = _vel_elem[_qp] - _normal_speed_elem * _normal;
  return _normal(_index) * _SM + vel_nonnormal(_index);

  // For some reason, the below expression doesn't give as good results as the
  // above one.
  // return _normal(_index) * (_SM - _normal_speed_elem) + _vel_elem[_qp](_index);
}

ADReal
PCNSFVMomentumHLLC::hllcNeighbor()
{
  auto vel_nonnormal = _vel_neighbor[_qp] - _normal_speed_neighbor * _normal;
  return _normal(_index) * _SM + vel_nonnormal(_index);

  // For some reason, the below expression doesn't give as good results as the
  // above one.
  // return _normal(_index) * (_SM - _normal_speed_neighbor) + _vel_elem[_qp](_index);
}

ADReal
PCNSFVMomentumHLLC::conservedVariableElem()
{
  return _eps_elem[_qp] * _rho_elem[_qp] * _vel_elem[_qp](_index);
}

ADReal
PCNSFVMomentumHLLC::conservedVariableNeighbor()
{
  return _eps_neighbor[_qp] * _rho_neighbor[_qp] * _vel_neighbor[_qp](_index);
}
