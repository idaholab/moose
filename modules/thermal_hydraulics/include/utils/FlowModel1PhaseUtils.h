//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"

namespace FlowModel1PhaseUtils
{

/**
 * Computes the primitive solution vector from the conservative solution vector
 *
 * @param[in] U   Conservative solution vector
 * @param[in] fp  Fluid properties object
 */
template <bool is_ad>
std::vector<GenericReal<is_ad>>
computePrimitiveSolutionVector(const std::vector<GenericReal<is_ad>> & U,
                               const SinglePhaseFluidProperties & fp)
{
  const auto & rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const auto & rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const auto & rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const auto & A = U[THM3Eqn::CONS_VAR_AREA];

  const auto rho = rhoA / A;
  const auto vel = rhouA / rhoA;
  const auto v = 1.0 / rho;
  const auto e = rhoEA / rhoA - 0.5 * vel * vel;
  const auto p = fp.p_from_v_e(v, e);
  const auto T = fp.T_from_v_e(v, e);

  std::vector<GenericReal<is_ad>> W(THM3Eqn::N_PRIM_VAR);
  W[THM3Eqn::PRIM_VAR_PRESSURE] = fp.p_from_v_e(v, e);
  W[THM3Eqn::PRIM_VAR_VELOCITY] = vel;
  W[THM3Eqn::PRIM_VAR_TEMPERATURE] = fp.T_from_v_e(v, e);

  return W;
}

/**
 * Computes the conservative solution vector from the primitive solution vector
 *
 * @param[in] W   Primitive solution vector
 * @param[in] A   Cross-sectional area
 * @param[in] fp  Fluid properties object
 */
template <bool is_ad>
std::vector<GenericReal<is_ad>>
computeConservativeSolutionVector(const std::vector<GenericReal<is_ad>> & W,
                                  const GenericReal<is_ad> & A,
                                  const SinglePhaseFluidProperties & fp)
{
  const auto & p = W[THM3Eqn::PRIM_VAR_PRESSURE];
  const auto & T = W[THM3Eqn::PRIM_VAR_TEMPERATURE];
  const auto & vel = W[THM3Eqn::PRIM_VAR_VELOCITY];

  const ADReal rho = fp.rho_from_p_T(p, T);
  const ADReal e = fp.e_from_p_rho(p, rho);
  const ADReal E = e + 0.5 * vel * vel;

  std::vector<GenericReal<is_ad>> U(THM3Eqn::N_CONS_VAR);
  U[THM3Eqn::CONS_VAR_RHOA] = rho * A;
  U[THM3Eqn::CONS_VAR_RHOUA] = U[THM3Eqn::CONS_VAR_RHOA] * vel;
  U[THM3Eqn::CONS_VAR_RHOEA] = U[THM3Eqn::CONS_VAR_RHOA] * E;
  U[THM3Eqn::CONS_VAR_AREA] = A;

  return U;
}

}
