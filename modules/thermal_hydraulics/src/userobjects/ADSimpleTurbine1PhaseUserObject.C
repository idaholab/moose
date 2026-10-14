//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSimpleTurbine1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "THMIndicesVACE.h"
#include "ADNumericalFlux3EqnBase.h"
#include "Numerics.h"
#include "THMUtils.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADSimpleTurbine1PhaseUserObject);

InputParameters
ADSimpleTurbine1PhaseUserObject::validParams()
{
  InputParameters params = ADJunctionParallelChannels1PhaseUserObject::validParams();
  params.addRequiredParam<FunctionName>(
      "on", "Function determining if turbine is operating (0=off, 1=on)");
  params.addRequiredParam<FunctionName>("W_dot", "Function specifying the power [W]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase turbine");

  return params;
}

ADSimpleTurbine1PhaseUserObject::ADSimpleTurbine1PhaseUserObject(const InputParameters & params)
  : ADJunctionParallelChannels1PhaseUserObject(params),
    _on_fn(getFunction("on")),
    _W_dot_fn(getFunction("W_dot"))
{
}

void
ADSimpleTurbine1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADJunctionParallelChannels1PhaseUserObject::computeFluxesAndResiduals(c);

  using std::pow;

  if ((c == 0) && THM::realToBool(_on_fn.value(_t, Point())))
  {
    const Real W_dot = _W_dot_fn.value(_t, Point());

    const auto & rhouV = _cached_junction_var_values[VolumeJunction1Phase::RHOUV_INDEX];
    const auto & rhovV = _cached_junction_var_values[VolumeJunction1Phase::RHOVV_INDEX];
    const auto & rhowV = _cached_junction_var_values[VolumeJunction1Phase::RHOWV_INDEX];

    const Point di = _dir[0];
    const ADRealVectorValue rhouV_vec(rhouV, rhovV, rhowV);

    // energy source
    const ADReal S_E = W_dot;

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
    const ADReal delta_p = p_in * (1 - pow((1 - W_dot / _rhouA[0] / h_in), (gamma / (gamma - 1))));

    const ADRealVectorValue S_M = delta_p * _A[0] * di;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] += S_M(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] += S_M(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] += S_M(2);
    _residual[VolumeJunction1Phase::RHOEV_INDEX] += S_E;
  }
}
