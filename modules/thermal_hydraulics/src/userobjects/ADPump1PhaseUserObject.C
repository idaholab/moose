//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPump1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndicesVACE.h"
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

  const auto & rhoV = _cached_junction_var_values[VolumeJunction1Phase::RHOV_INDEX];

  const ADRealVectorValue di = _dir[0];
  const ADReal rho = rhoV / _volume;

  // compute momentum and energy source terms
  const ADRealVectorValue S_momentum = 0.5 * rho * _g * _head * _A_ref * di;
  // The energy source is the work of the head on the flow through this connection, so that the
  // pump adds m_dot * g * head over its two connections. The velocity of the junction volume is
  // not used because the head raises it above the through-flow velocity.
  const ADReal S_energy = 0.5 * _g * _head * _flux[c][THMVACE1D::MASS];

  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);
  _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;
}
