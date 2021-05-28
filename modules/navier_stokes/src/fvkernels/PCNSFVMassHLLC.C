//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVMassHLLC.h"

registerMooseObject("NavierStokesApp", PCNSFVMassHLLC);

InputParameters
PCNSFVMassHLLC::validParams()
{
  InputParameters params = PCNSFVHLLC::validParams();
  params.addClassDescription("Implements the mass flux portion of the porous HLLC discretization.");
  return params;
}

PCNSFVMassHLLC::PCNSFVMassHLLC(const InputParameters & params) : PCNSFVHLLC(params) {}

ADReal
PCNSFVMassHLLC::fluxElem()
{
  return _normal_speed_elem * _eps_elem[_qp] * _rho_elem[_qp];
}

ADReal
PCNSFVMassHLLC::fluxNeighbor()
{
  return _normal_speed_neighbor * _eps_neighbor[_qp] * _rho_neighbor[_qp];
}

ADReal
PCNSFVMassHLLC::hllcElem()
{
  return 1;
}

ADReal
PCNSFVMassHLLC::hllcNeighbor()
{
  return 1;
}

ADReal
PCNSFVMassHLLC::conservedVariableElem()
{
  return _eps_elem[_qp] * _rho_elem[_qp];
}

ADReal
PCNSFVMassHLLC::conservedVariableNeighbor()
{
  return _eps_neighbor[_qp] * _rho_neighbor[_qp];
}
