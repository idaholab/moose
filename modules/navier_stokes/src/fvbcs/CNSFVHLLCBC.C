//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCBC.h"
#include "CNSFVHLLC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

InputParameters
CNSFVHLLCBC::validParams()
{
  return CNSFVHLLCBCBase::validParams();
}

CNSFVHLLCBC::CNSFVHLLCBC(const InputParameters & parameters) : CNSFVHLLCBCBase(parameters) {}

ADReal
CNSFVHLLCBC::computeQpResidual()
{
  _normal_speed_elem = _normal * _vel_elem[_qp];
  preComputeWaveSpeed();

  auto wave_speeds = CNSFVHLLCBase::waveSpeed(hllcData(), _normal);

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
}
