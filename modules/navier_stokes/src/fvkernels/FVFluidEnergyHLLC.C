//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluidEnergyHLLC.h"
#include "NS.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVFluidEnergyHLLC);

InputParameters
CNSFVFluidEnergyHLLC::validParams()
{
  InputParameters params = CNSFVHLLC::validParams();
  return params;
}

CNSFVFluidEnergyHLLC::CNSFVFluidEnergyHLLC(const InputParameters & params)
  : CNSFVHLLC(params),
    _h_elem(getADMaterialProperty<Real>(nms::enthalpy)),
    _h_neighbor(getNeighborADMaterialProperty<Real>(nms::enthalpy))
{
}

ADReal
CNSFVFluidEnergyHLLC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp] * _h_elem[_qp];
}

ADReal
CNSFVFluidEnergyHLLC::fluxNeighbor()
{
  return _normal_speed_neighbor * _rho_neighbor[_qp] * _h_neighbor[_qp];
}

ADReal
CNSFVFluidEnergyHLLC::hllcElem()
{
  return _rhoE_elem[_qp] / _rho_elem[_qp] + (_SM - _normal_speed_elem) * (_SM + _pressure_elem[_qp] / _rho_elem[_qp] /
                                                                (_SL - _normal_speed_elem));
}

ADReal
CNSFVFluidEnergyHLLC::hllcNeighbor()
{
  return _rhoE_neighbor[_qp] / _rho_neighbor[_qp] +
         (_SM - _normal_speed_neighbor) *
             (_SM + _pressure_neighbor[_qp] / _rho_neighbor[_qp] / (_SR - _normal_speed_neighbor));
}

ADReal
CNSFVFluidEnergyHLLC::conservedVariableElem()
{
  return _rhoE_elem[_qp];
}

ADReal
CNSFVFluidEnergyHLLC::conservedVariableNeighbor()
{
  return _rhoE_neighbor[_qp];
}
