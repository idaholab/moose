//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVHLLCBC.h"
#include "PCNSFVHLLC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

InputParameters
PCNSFVHLLCBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
  return params;
}

PCNSFVHLLCBC::PCNSFVHLLCBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _specific_internal_energy_elem(getADMaterialProperty<Real>(NS::specific_internal_energy)),
    _vel_elem(getADMaterialProperty<RealVectorValue>(NS::velocity)),
    _speed_elem(getADMaterialProperty<Real>(NS::speed)),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _rho_et_elem(getADMaterialProperty<Real>(NS::total_energy_density)),
    _ht_elem(getADMaterialProperty<Real>(NS::specific_total_enthalpy)),
    _eps_elem(getMaterialProperty<Real>(NS::porosity))
{
}

ADReal
PCNSFVHLLCBC::computeQpResidual()
{

  _normal_speed_elem = _normal * _vel_elem[_qp];
  preComputeWaveSpeed();

  auto wave_speeds = PCNSFVHLLC::waveSpeed(_rho_elem[_qp],
                                           _vel_elem[_qp],
                                           _specific_internal_energy_elem[_qp],
                                           _eps_elem[_qp],
                                           _rho_boundary,
                                           _vel_boundary,
                                           _specific_internal_energy_boundary,
                                           _eps_boundary,
                                           _fluid,
                                           _normal);
  _SL = std::move(wave_speeds[0]);
  _SM = std::move(wave_speeds[1]);
  _SR = std::move(wave_speeds[2]);

  if (_SL >= 0)
    return fluxElem();
  else if (_SR <= 0)
    return fluxBoundary();
  else
  {
    if (_SM >= 0)
    {
      ADReal f = _eps_elem[_qp] * _rho_elem[_qp] * (_SL - _normal_speed_elem) / (_SL - _SM);
      return fluxElem() + _SL * (f * hllcElem() - conservedVariableElem());
    }
    else
    {
      ADReal f = _eps_boundary * _rho_boundary * (_SR - _normal_speed_boundary) / (_SR - _SM);
      return fluxBoundary() + _SR * (f * hllcBoundary() - conservedVariableBoundary());
    }
  }
  mooseError("Should never get here");
  return 0;
}
