//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPump1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "VolumeJunction1Phase.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADPump1PhaseUserObject);

InputParameters
ADPump1PhaseUserObject::validParams()
{
  InputParameters params = ADVolumeJunction1PhaseUserObject::validParams();

  params.addRequiredParam<Real>("head", "Pump head, [m]");
  params.addRequiredParam<Real>("gravity_magnitude", "Gravity constant, [m/s^2]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase pump");

  params.declareControllable("head");

  return params;
}

ADPump1PhaseUserObject::ADPump1PhaseUserObject(const InputParameters & params)
  : ADVolumeJunction1PhaseUserObject(params),
    _head(getParam<Real>("head")),
    _g(getParam<Real>("gravity_magnitude"))
{
}

void
ADPump1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  const ADRealVectorValue di = _dir[0];
  const ADRealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);

  // compute momentum and energy source terms
  const ADRealVectorValue S_momentum = 0.5 * (_rhoV[0] / _volume) * _g * _head * _A_ref * di;
  const ADReal S_energy = 0.5 * ((rhouV_vec * di) / _volume) * _g * _head * _A_ref;
  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);
  _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;
}
