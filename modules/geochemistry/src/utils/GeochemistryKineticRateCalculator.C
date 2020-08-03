//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryKineticRateCalculator.h"

namespace GeochemistryKineticRateCalculator
{
void
calculateRate(const std::vector<Real> & promoting_indices,
              const KineticRateUserDescription & description,
              const std::vector<std::string> & basis_species_name,
              const std::vector<bool> & basis_species_gas,
              const std::vector<Real> & basis_molality,
              const std::vector<Real> & basis_activity,
              const std::vector<bool> & basis_activity_known,
              const std::vector<std::string> & eqm_species_name,
              const std::vector<bool> & eqm_species_gas,
              const std::vector<Real> & eqm_molality,
              const std::vector<Real> & eqm_activity,
              const DenseMatrix<Real> & eqm_stoichiometry,
              Real kin_moles,
              Real kin_species_molecular_weight,
              Real log10K,
              Real log10_activity_product,
              const DenseMatrix<Real> & kin_stoichiometry,
              unsigned kin,
              Real temp_degC,
              Real & rate,
              Real & drate_dkin,
              std::vector<Real> & drate_dmol)
{
  const unsigned num_basis = basis_species_name.size();
  const unsigned num_eqm = eqm_species_name.size();

  if (num_basis + num_eqm != promoting_indices.size())
    mooseError("kinetic_rate: promoting_indices incorrectly sized ", promoting_indices.size());
  if (!(num_basis == basis_species_gas.size() && num_basis == basis_molality.size() &&
        num_basis == basis_activity.size() && num_basis == basis_activity_known.size() &&
        num_basis == drate_dmol.size()))
    mooseError("kinetic_rate: incorrectly sized basis-species vectors ",
               num_basis,
               " ",
               basis_species_gas.size(),
               " ",
               basis_molality.size(),
               " ",
               basis_activity.size(),
               " ",
               basis_activity_known.size(),
               " ",
               drate_dmol.size());
  if (!(num_eqm == eqm_species_gas.size() && num_eqm == eqm_molality.size() &&
        num_eqm == eqm_activity.size()))
    mooseError("kinetic_rate: incorrectly sized equilibrium-species vectors ",
               num_eqm,
               " ",
               eqm_species_gas.size(),
               " ",
               eqm_molality.size(),
               " ",
               eqm_activity.size());
  if (!(eqm_stoichiometry.m() == num_eqm && eqm_stoichiometry.n() == num_basis))
    mooseError("kinetic_rate: incorrectly sized eqm stoichiometry matrix ",
               eqm_stoichiometry.m(),
               " ",
               eqm_stoichiometry.n());
  if (!(kin_stoichiometry.m() > kin && kin_stoichiometry.n() == num_basis))
    mooseError("kinetic_rate: incorrectly sized kinetic stoichiometry matrix ",
               kin_stoichiometry.m(),
               " ",
               kin_stoichiometry.n());

  rate = description.intrinsic_rate_constant;
  rate *= description.area_quantity;
  if (description.multiply_by_mass)
    rate *= kin_moles * kin_species_molecular_weight;

  for (unsigned i = 0; i < num_basis; ++i)
  {
    if (promoting_indices[i] == 0.0)
      continue;
    if (basis_species_gas[i] || basis_species_name[i] == "H+" || basis_species_name[i] == "OH-")
      rate *= std::pow(basis_activity[i], promoting_indices[i]);
    else
      rate *= std::pow(basis_molality[i], promoting_indices[i]);
  }
  for (unsigned j = 0; j < num_eqm; ++j)
  {
    const unsigned index = num_basis + j;
    if (promoting_indices[index] == 0.0)
      continue;
    if (eqm_species_gas[j] || eqm_species_name[j] == "H+" || eqm_species_name[j] == "OH-")
      rate *= std::pow(eqm_activity[j], promoting_indices[index]);
    else
      rate *= std::pow(eqm_molality[j], promoting_indices[index]);
  }
  const Real ap_over_k = std::pow(10.0, log10_activity_product - log10K);
  const Real theta_term = std::pow(ap_over_k, description.theta);
  rate *= std::pow(std::abs(1.0 - theta_term), description.eta);
  rate *= std::exp(
      description.activation_energy / GeochemistryConstants::GAS_CONSTANT *
      (description.one_over_T0 - 1.0 / (temp_degC + GeochemistryConstants::CELSIUS_TO_KELVIN)));
  if (ap_over_k > 1.0)
    rate = -rate;

  if (description.multiply_by_mass)
    drate_dkin = rate / kin_moles;
  else
    drate_dkin = 0.0;

  // In the following, all derivatives of activity coefficients are ignored
  for (unsigned i = 0; i < num_basis; ++i)
  {
    if (promoting_indices[i] == 0.0)
      drate_dmol[i] = 0.0;
    else if (basis_species_gas[i]) // molality is undefined
      drate_dmol[i] = 0.0;
    else
      drate_dmol[i] = promoting_indices[i] * rate / basis_molality[i];
  }
  for (unsigned j = 0; j < num_eqm; ++j)
  {
    const unsigned index = num_basis + j;
    if (promoting_indices[index] == 0.0)
      continue;
    for (unsigned i = 1; i < num_basis; ++i) // deriv of water activity is ignored
      if (!(basis_species_gas[i] || eqm_stoichiometry(j, i) == 0.0))
        drate_dmol[i] +=
            promoting_indices[index] * rate * eqm_stoichiometry(j, i) / basis_molality[i];
  }
  Real deriv_ap_term =
      description.eta * rate / std::abs(1 - theta_term) * (-description.theta) * theta_term;
  if (theta_term > 1.0)
    deriv_ap_term = -deriv_ap_term;
  for (unsigned i = 1; i < num_basis; ++i)
    if (!(basis_species_gas[i] || kin_stoichiometry(kin, i) == 0.0))
      drate_dmol[i] += deriv_ap_term * kin_stoichiometry(kin, i) / basis_molality[i];
}
}
