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
#include "THMIndicesVACE.h"
#include "MooseVariable.h"

#include "libmesh/elem.h"

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
  const auto & rhoA = U[THMVACE1D::RHOA];
  const auto & rhouA = U[THMVACE1D::RHOUA];
  const auto & rhoEA = U[THMVACE1D::RHOEA];
  const auto & A = U[THMVACE1D::AREA];

  const auto rho = rhoA / A;
  const auto vel = rhouA / rhoA;
  const auto v = 1.0 / rho;
  const auto e = rhoEA / rhoA - 0.5 * vel * vel;
  const auto p = fp.p_from_v_e(v, e);
  const auto T = fp.T_from_v_e(v, e);

  std::vector<GenericReal<is_ad>> W(THMVACE1D::N_PRIM_VARS);
  W[THMVACE1D::PRESSURE] = fp.p_from_v_e(v, e);
  W[THMVACE1D::VELOCITY] = vel;
  W[THMVACE1D::TEMPERATURE] = fp.T_from_v_e(v, e);

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
  const auto & p = W[THMVACE1D::PRESSURE];
  const auto & T = W[THMVACE1D::TEMPERATURE];
  const auto & vel = W[THMVACE1D::VELOCITY];

  const ADReal rho = fp.rho_from_p_T(p, T);
  const ADReal e = fp.e_from_p_rho(p, rho);
  const ADReal E = e + 0.5 * vel * vel;

  std::vector<GenericReal<is_ad>> U(THMVACE1D::N_FLUX_INPUTS);
  U[THMVACE1D::RHOA] = rho * A;
  U[THMVACE1D::RHOUA] = U[THMVACE1D::RHOA] * vel;
  U[THMVACE1D::RHOEA] = U[THMVACE1D::RHOA] * E;
  U[THMVACE1D::AREA] = A;

  return U;
}

/**
 * Gets the elemental conservative solution vector
 *
 * @param[in] elem         Element
 * @param[in] U_vars       Vector of conservative variable pointers
 * @param[in] is_implicit  Is implicit?
 */
template <bool is_ad>
std::vector<GenericReal<is_ad>>
getElementalSolutionVector(const Elem * elem,
                           const std::vector<MooseVariable *> & U_vars,
                           bool is_implicit)
{
  mooseAssert(elem, "The supplied element is a nullptr.");

  std::vector<GenericReal<is_ad>> U(THMVACE1D::N_FLUX_INPUTS, 0.0);

  if (is_implicit)
  {
    for (unsigned int i = 0; i < THMVACE1D::N_FLUX_INPUTS; i++)
    {
      mooseAssert(U_vars[i], "The supplied variable is a nullptr.");
      U[i] = U_vars[i]->getElementalValue(elem);
    }

    std::vector<dof_id_type> dof_indices;

    const std::vector<unsigned int> ind = {THMVACE1D::RHOA, THMVACE1D::RHOUA, THMVACE1D::RHOEA};
    for (unsigned int j = 0; j < ind.size(); j++)
    {
      const auto i = ind[j];
      U_vars[i]->dofMap().dof_indices(elem, dof_indices, U_vars[i]->number());
      Moose::derivInsert(U[i].derivatives(), dof_indices[0], 1.0);
    }
  }
  else
  {
    for (unsigned int i = 0; i < THMVACE1D::N_FLUX_INPUTS; i++)
      U[i] = U_vars[i]->getElementalValueOld(elem);
  }

  return U;
}
}
