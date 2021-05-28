//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVFluidEnergyHLLC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVFluidEnergyHLLC);

InputParameters
PCNSFVFluidEnergyHLLC::validParams()
{
  InputParameters params = PCNSFVHLLC::validParams();
  params.addClassDescription(
      "Implements the fluid energy flux portion of the porous HLLC discretization.");
  return params;
}

PCNSFVFluidEnergyHLLC::PCNSFVFluidEnergyHLLC(const InputParameters & params)
  : PCNSFVHLLC(params),
    _ht_elem(getADMaterialProperty<Real>(NS::specific_total_enthalpy)),
    _ht_neighbor(getNeighborADMaterialProperty<Real>(NS::specific_total_enthalpy))
{
}

ADReal
PCNSFVFluidEnergyHLLC::fluxElem()
{
  return _normal_speed_elem * _eps_elem[_qp] * _rho_elem[_qp] * _ht_elem[_qp];
}

ADReal
PCNSFVFluidEnergyHLLC::fluxNeighbor()
{
  return _normal_speed_neighbor * _eps_neighbor[_qp] * _rho_neighbor[_qp] * _ht_neighbor[_qp];
}

ADReal
PCNSFVFluidEnergyHLLC::hllcElem()
{
  return _rho_et_elem[_qp] / _rho_elem[_qp] +
         (_SM - _normal_speed_elem) *
             (_SM + _pressure_elem[_qp] / _rho_elem[_qp] / (_SL - _normal_speed_elem));
}

ADReal
PCNSFVFluidEnergyHLLC::hllcNeighbor()
{
  return _rho_et_neighbor[_qp] / _rho_neighbor[_qp] +
         (_SM - _normal_speed_neighbor) *
             (_SM + _pressure_neighbor[_qp] / _rho_neighbor[_qp] / (_SR - _normal_speed_neighbor));
}

ADReal
PCNSFVFluidEnergyHLLC::conservedVariableElem()
{
  return _eps_elem[_qp] * _rho_et_elem[_qp];
}

ADReal
PCNSFVFluidEnergyHLLC::conservedVariableNeighbor()
{
  return _eps_neighbor[_qp] * _rho_et_neighbor[_qp];
}
