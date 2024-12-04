//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IdealRealGasMixtureFluidProperties.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndicesGasMix.h"
// #include "MooseVariable.h"

// #include "libmesh/elem.h"

namespace FlowModelGasMixUtils
{

/**
 * Computes the primitive solution vector from the conservative solution vector
 *
 * @param[in] U   Conservative solution vector
 * @param[in] fp  Fluid properties object
 */
template <bool is_ad>
std::vector<GenericReal<is_ad>>
computePrimitiveSolution(const std::vector<GenericReal<is_ad>> & U,
                         const IdealRealGasMixtureFluidProperties & fp)
{
  const auto & xirhoA = U[THMGasMix1D::XIRHOA];
  const auto & rhoA = U[THMGasMix1D::RHOA];
  const auto & rhouA = U[THMGasMix1D::RHOUA];
  const auto & rhoEA = U[THMGasMix1D::RHOEA];
  const auto & A = U[THMGasMix1D::AREA];

  const auto xi = xirhoA / rhoA;
  const auto rho = rhoA / A;
  const auto vel = rhouA / rhoA;
  const auto v = 1.0 / rho;
  const auto e = rhoEA / rhoA - 0.5 * vel * vel;
  const auto p = fp.p_from_v_e(v, e, {xi});
  const auto T = fp.T_from_v_e(v, e, {xi});

  std::vector<GenericReal<is_ad>> W(THMGasMix1D::N_PRIM_VARS);
  W[THMGasMix1D::MASS_FRACTION] = xi;
  W[THMGasMix1D::PRESSURE] = p;
  W[THMGasMix1D::VELOCITY] = vel;
  W[THMGasMix1D::TEMPERATURE] = T;

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
computeConservativeSolution(const std::vector<GenericReal<is_ad>> & W,
                            const GenericReal<is_ad> & A,
                            const IdealRealGasMixtureFluidProperties & fp)
{
  const auto & xi = W[THMGasMix1D::MASS_FRACTION];
  const auto & p = W[THMGasMix1D::PRESSURE];
  const auto & T = W[THMGasMix1D::TEMPERATURE];
  const auto & vel = W[THMGasMix1D::VELOCITY];

  const auto rho = fp.rho_from_p_T(p, T, {xi});
  const auto rhoA = rho * A;
  const auto e = fp.e_from_p_rho(p, rho, {xi});
  const auto E = e + 0.5 * vel * vel;

  std::vector<GenericReal<is_ad>> U(THMGasMix1D::N_FLUX_INPUTS);
  U[THMGasMix1D::XIRHOA] = xi * rhoA;
  U[THMGasMix1D::RHOA] = rhoA;
  U[THMGasMix1D::RHOUA] = rhoA * vel;
  U[THMGasMix1D::RHOEA] = rhoA * E;
  U[THMGasMix1D::AREA] = A;

  return U;
}

/**
 * Computes the numerical flux vector from the primitive solution vector
 *
 * @param[in] W   Primitive solution vector
 * @param[in] A   Cross-sectional area
 * @param[in] fp  Fluid properties object
 */
template <bool is_ad>
std::vector<GenericReal<is_ad>>
computeFluxFromPrimitive(const std::vector<GenericReal<is_ad>> & W,
                         const GenericReal<is_ad> & A,
                         const IdealRealGasMixtureFluidProperties & fp)
{
  const auto & xi = W[THMGasMix1D::MASS_FRACTION];
  const auto & p = W[THMGasMix1D::PRESSURE];
  const auto & T = W[THMGasMix1D::TEMPERATURE];
  const auto & vel = W[THMGasMix1D::VELOCITY];

  const auto rho = fp.rho_from_p_T(p, T, {xi});
  const auto e = fp.e_from_p_rho(p, rho, {xi});
  const auto E = e + 0.5 * vel * vel;

  std::vector<ADReal> F(THMGasMix1D::N_FLUX_OUTPUTS, 0.0);
  F[THMGasMix1D::SPECIES] = xi * rho * vel * A;
  F[THMGasMix1D::MASS] = rho * vel * A;
  F[THMGasMix1D::MOMENTUM] = (rho * vel * vel + p) * A;
  F[THMGasMix1D::ENERGY] = vel * (rho * E + p) * A;

  return F;
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

  std::vector<GenericReal<is_ad>> U(THMGasMix1D::N_FLUX_INPUTS, 0.0);

  if (is_implicit)
  {
    for (const auto i : make_range(THMGasMix1D::N_FLUX_INPUTS))
    {
      mooseAssert(U_vars[i], "The supplied variable is a nullptr.");
      U[i] = U_vars[i]->getElementalValue(elem);
    }

    std::vector<dof_id_type> dof_indices;

    const std::vector<unsigned int> ind = {
        THMGasMix1D::XIRHOA, THMGasMix1D::RHOA, THMGasMix1D::RHOUA, THMGasMix1D::RHOEA};
    for (const auto j : index_range(ind))
    {
      const auto i = ind[j];
      U_vars[i]->dofMap().dof_indices(elem, dof_indices, U_vars[i]->number());
      Moose::derivInsert(U[i].derivatives(), dof_indices[0], 1.0);
    }
  }
  else
  {
    for (const auto i : make_range(THMGasMix1D::N_FLUX_INPUTS))
      U[i] = U_vars[i]->getElementalValueOld(elem);
  }

  return U;
}

template <bool is_ad>
GenericReal<is_ad>
computeSecondaryMoleFraction(const GenericReal<is_ad> & xi_secondary,
                             const VaporMixtureFluidProperties & fp)
{
  mooseAssert(fp.getNumberOfSecondaryVapors() == 1,
              "This function assumes there is a single secondary fluid.");
  const SinglePhaseFluidProperties & fp_primary = fp.getPrimaryFluidProperties();
  const SinglePhaseFluidProperties & fp_secondary = fp.getSecondaryFluidProperties();

  const GenericReal<is_ad> xi_primary = 1 - xi_secondary;

  const GenericReal<is_ad> moles_primary = xi_primary / fp_primary.molarMass();
  const GenericReal<is_ad> moles_secondary = xi_secondary / fp_secondary.molarMass();

  return moles_secondary / (moles_primary + moles_secondary);
}
}
