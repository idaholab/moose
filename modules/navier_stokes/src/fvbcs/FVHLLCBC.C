//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVHLLCBC.h"
#include "FVHLLC.h"
#include "NS.h"

namespace nms = NS;

InputParameters
CNSFVHLLCBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredParam<UserObjectName>(nms::fluid, "Fluid userobject");
  return params;
}

CNSFVHLLCBC::CNSFVHLLCBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(nms::fluid)),
    _specific_internal_energy_elem(getADMaterialProperty<Real>(nms::specific_internal_energy)),
    _vel_elem(getADMaterialProperty<RealVectorValue>(nms::velocity)),
    _speed_elem(getADMaterialProperty<Real>(nms::speed)),
    _rho_elem(getADMaterialProperty<Real>(nms::density)),
    _pressure_elem(getADMaterialProperty<Real>(nms::pressure)),
    _rho_et_elem(getADMaterialProperty<Real>(nms::total_energy_density))
{
}

ADReal
CNSFVHLLCBC::computeQpResidual()
{
  _normal_speed_elem = _normal * _vel_elem[_qp];
  preComputeWaveSpeed();

  const auto & wave_speeds = CNSFVHLLC::waveSpeed(_rho_elem[_qp],
                                               _vel_elem[_qp],
                                               _specific_internal_energy_elem[_qp],
                                               _rho_boundary,
                                               _vel_boundary,
                                               _specific_internal_energy_boundary,
                                               _fluid,
                                               _normal);
  _SL = wave_speeds[0];
  _SM = wave_speeds[1];
  _SR = wave_speeds[2];

  if (_SL >= 0)
    return fluxElem();
  else if (_SR <= 0)
    return fluxBoundary();
  else
  {
    if (_SM >= 0)
    {
      ADReal f = _rho_elem[_qp] * (_SL - _normal_speed_elem) / (_SL - _SM);
      return fluxElem() + _SL * (f * hllcElem() - conservedVariableElem());
    }
    else
    {
      ADReal f = _rho_boundary * (_SR - _normal_speed_boundary) / (_SR - _SM);
      return fluxBoundary() + _SR * (f * hllcBoundary() - conservedVariableBoundary());
    }
  }
  mooseError("Should never get here");
  return 0;

  return 0;
}
