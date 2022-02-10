//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSimpleTurbine1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "THMIndices3Eqn.h"
#include "ADNumericalFlux3EqnBase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADSimpleTurbine1PhaseUserObject);

InputParameters
ADSimpleTurbine1PhaseUserObject::validParams()
{
  InputParameters params = ADJunctionParallelChannels1PhaseUserObject::validParams();
  params.addRequiredParam<bool>("on", "Flag determining if turbine is operating or not");
  params.addRequiredParam<Real>("W_dot", "Power, [W]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase turbine");

  params.declareControllable("W_dot on");

  return params;
}

ADSimpleTurbine1PhaseUserObject::ADSimpleTurbine1PhaseUserObject(const InputParameters & params)
  : ADJunctionParallelChannels1PhaseUserObject(params),
    _on(getParam<bool>("on")),
    _W_dot(getParam<Real>("W_dot"))
{
}

void
ADSimpleTurbine1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADJunctionParallelChannels1PhaseUserObject::computeFluxesAndResiduals(c);

  if ((c == 0) && _on)
  {
    const Point di = _dir[0];
    const ADRealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);

    // energy source
    const ADReal S_E = _W_dot;

    // momentum source
    const ADReal v_in = THM::v_from_rhoA_A(_rhoA[0], _A[0]);

    const ADReal rhouA2 = _rhouA[0] * _rhouA[0];
    const ADReal e_in = _rhoEA[0] / _rhoA[0] - 0.5 * rhouA2 / (_rhoA[0] * _rhoA[0]);

    const ADReal cp = _fp.cp_from_v_e(v_in, e_in);
    const ADReal cv = _fp.cv_from_v_e(v_in, e_in);
    const ADReal gamma = cp / cv;
    const ADReal p_in = _fp.p_from_v_e(v_in, e_in);
    const ADReal T_in = _fp.T_from_v_e(v_in, e_in);
    const ADReal h_in = _fp.h_from_p_T(p_in, T_in);
    const ADReal delta_p =
        p_in * (1 - std::pow((1 - _W_dot / _rhouA[0] / h_in), (gamma / (gamma - 1))));

    const ADRealVectorValue S_M = delta_p * _A[0] * di;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] += S_M(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] += S_M(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] += S_M(2);
    _residual[VolumeJunction1Phase::RHOEV_INDEX] += S_E;
  }
}
