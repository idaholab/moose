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
              const std::vector<Real> & promoting_monod_indices,
              const std::vector<Real> & promoting_half_saturation,
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
  if (num_basis + num_eqm != promoting_monod_indices.size())
    mooseError("kinetic_rate: promoting_monod_indices incorrectly sized ",
               promoting_monod_indices.size());
  if (num_basis + num_eqm != promoting_half_saturation.size())
    mooseError("kinetic_rate: promoting_half_saturation incorrectly sized ",
               promoting_half_saturation.size());
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

  const Real kin_molality = kin_moles / basis_molality[0];
  const Real dkin_molality_dkin_moles = 1.0 / basis_molality[0];
  const Real dkin_molality_dnw = -kin_molality / basis_molality[0];
  if (description.kinetic_molal_index != 0.0)
    rate *= std::pow(kin_molality, description.kinetic_molal_index);
  if (description.kinetic_monod_index != 0.0)
    rate /=
        std::pow(std::pow(kin_molality, description.kinetic_molal_index) +
                     std::pow(description.kinetic_half_saturation, description.kinetic_molal_index),
                 description.kinetic_monod_index);

  // promoting species numerators
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

  // promoting species denominators: the monod terms
  for (unsigned i = 0; i < num_basis; ++i)
  {
    if (promoting_monod_indices[i] == 0.0)
      continue;
    if (basis_species_gas[i] || basis_species_name[i] == "H+" || basis_species_name[i] == "OH-")
      rate /= std::pow(std::pow(basis_activity[i], promoting_indices[i]) +
                           std::pow(promoting_half_saturation[i], promoting_indices[i]),
                       promoting_monod_indices[i]);
    else
      rate /= std::pow(std::pow(basis_molality[i], promoting_indices[i]) +
                           std::pow(promoting_half_saturation[i], promoting_indices[i]),
                       promoting_monod_indices[i]);
  }
  for (unsigned j = 0; j < num_eqm; ++j)
  {
    const unsigned index = num_basis + j;
    if (promoting_monod_indices[index] == 0.0)
      continue;
    if (eqm_species_gas[j] || eqm_species_name[j] == "H+" || eqm_species_name[j] == "OH-")
      rate /= std::pow(std::pow(eqm_activity[j], promoting_indices[index]) +
                           std::pow(promoting_half_saturation[index], promoting_indices[index]),
                       promoting_monod_indices[index]);
    else
      rate /= std::pow(std::pow(eqm_molality[j], promoting_indices[index]) +
                           std::pow(promoting_half_saturation[index], promoting_indices[index]),
                       promoting_monod_indices[index]);
  }

  // temperature dependence
  rate *= std::exp(
      description.activation_energy / GeochemistryConstants::GAS_CONSTANT *
      (description.one_over_T0 - 1.0 / (temp_degC + GeochemistryConstants::CELSIUS_TO_KELVIN)));

  // dependence on activity and equilibrium constant
  const Real log10K_bio = log10K - description.energy_captured /
                                       GeochemistryConstants::GAS_CONSTANT /
                                       (temp_degC + GeochemistryConstants::CELSIUS_TO_KELVIN) /
                                       GeochemistryConstants::LOGTEN;
  const Real ap_over_k = std::pow(10.0, log10_activity_product - log10K_bio);
  const Real theta_term = std::pow(ap_over_k, description.theta);
  switch (description.direction)
  {
    case DirectionChoiceEnum::BOTH:
      if (ap_over_k > 1.0) // precipitation
        rate = -rate;
      // leave the sign of rate unchanged for dissolution
      break;
    case DirectionChoiceEnum::DISSOLUTION:
      if (ap_over_k > 1.0) // precipitation
        rate = 0.0;
      // leave the sign of rate unchanged for dissolution
      break;
    case DirectionChoiceEnum::PRECIPITATION:
      if (ap_over_k > 1.0) // precipitation
        rate = -rate;
      else // dissolution
        rate = 0.0;
      break;
    case DirectionChoiceEnum::RAW:
      break; // no dependence on 1 - ap_over_k
    case DirectionChoiceEnum::DEATH:
      break; // no dependence on 1 - ap_over_k
  }
  // const Real rate_no_theta_term = rate; // needed for derivative calcs when theta_term == 1
  rate *= (description.eta == 0.0)
              ? 1.0
              : ((theta_term == 1.0) ? 0.0 : std::pow(std::abs(1.0 - theta_term), description.eta));

  for (unsigned i = 0; i < num_basis; ++i)
    drate_dmol[i] = 0.0;
  drate_dkin = 0.0;

  // derivatives with respect to the kinetic species moles
  if (description.multiply_by_mass)
    drate_dkin += rate / kin_moles;

  if (description.kinetic_molal_index != 0.0)
  {
    const Real d_by_dkin_molality = description.kinetic_molal_index * rate / kin_molality;
    drate_dkin += d_by_dkin_molality * dkin_molality_dkin_moles;
    drate_dmol[0] += d_by_dkin_molality * dkin_molality_dnw;
  }
  if (!(description.kinetic_molal_index == 0 || description.kinetic_monod_index == 0.0))
  {
    const Real d_by_dkin_molality =
        -description.kinetic_monod_index * description.kinetic_molal_index *
        std::pow(kin_molality, description.kinetic_molal_index - 1) * rate /
        (std::pow(kin_molality, description.kinetic_molal_index) +
         std::pow(description.kinetic_half_saturation, description.kinetic_molal_index));
    drate_dkin += d_by_dkin_molality * dkin_molality_dkin_moles;
    drate_dmol[0] += d_by_dkin_molality * dkin_molality_dnw;
  }

  // Derivatives of the promoting-species numerators (ignore all derivatives of activity
  // coefficients), so
  // d(molality^P) / dmolality = P * molality^P / molality  , and
  // d(activity^P)/d(molality) = P activity^(P-1) d(activity)/d(molality) = P activity^(P-1)
  // activity_product  = P * activity^P / molality
  for (unsigned i = 0; i < num_basis; ++i)
  {
    if (promoting_indices[i] == 0.0)
      continue;
    else if (basis_species_gas[i]) // molality is undefined
      continue;
    else
      drate_dmol[i] += promoting_indices[i] * rate / basis_molality[i];
  }
  for (unsigned j = 0; j < num_eqm; ++j)
  {
    // d(eqm^P)/d(basis_i) = P eqm^P / eqm * d(eqm)/d(basis_i) = P eqm^P * stoi / basis_i
    const unsigned index = num_basis + j;
    if (promoting_indices[index] == 0.0)
      continue;
    for (unsigned i = 1; i < num_basis; ++i) // deriv of water activity is ignored
      if (!(basis_species_gas[i] || eqm_stoichiometry(j, i) == 0.0))
        drate_dmol[i] +=
            promoting_indices[index] * rate * eqm_stoichiometry(j, i) / basis_molality[i];
  }
  // Derivatives of the promoting-species denominators (ignore all derivatives of activity
  // coefficients) so
  // d((molality^P + K)^(-u))/d(molality) = -u (molality^P + K)^(-u) / (molality^P + K) * P *
  // molality^P / molality
  // d((activity^P + K)^(-u))/d(molality) = -u (activity^P + K)^(-u) /
  // (activity^P + K) * P * activity^P / molality
  for (unsigned i = 0; i < num_basis; ++i)
  {
    if (promoting_indices[i] == 0.0 || promoting_monod_indices[i] == 0.0)
      continue;
    else if (basis_species_gas[i]) // molality is undefined
      continue;
    else if (basis_species_name[i] == "H+" || basis_species_name[i] == "OH-")
      drate_dmol[i] -= promoting_monod_indices[i] * promoting_indices[i] *
                       std::pow(basis_activity[i], promoting_indices[i]) * rate /
                       basis_molality[i] /
                       (std::pow(basis_activity[i], promoting_indices[i]) +
                        std::pow(promoting_half_saturation[i], promoting_indices[i]));
    else
      drate_dmol[i] -= promoting_monod_indices[i] * promoting_indices[i] *
                       std::pow(basis_molality[i], promoting_indices[i] - 1) * rate /
                       (std::pow(basis_molality[i], promoting_indices[i]) +
                        std::pow(promoting_half_saturation[i], promoting_indices[i]));
  }
  for (unsigned j = 0; j < num_eqm; ++j)
  {
    const unsigned index = num_basis + j;
    if (promoting_indices[index] == 0.0 || promoting_monod_indices[index] == 0.0)
      continue;
    for (unsigned i = 1; i < num_basis; ++i) // deriv of water activity is ignored
    {
      if (basis_species_gas[i] || eqm_stoichiometry(j, i) == 0.0)
        continue;
      else if (eqm_species_gas[j] || eqm_species_name[j] == "H+" || eqm_species_name[j] == "OH-")
        drate_dmol[i] -= promoting_monod_indices[index] * promoting_indices[index] *
                         std::pow(eqm_activity[j], promoting_indices[index]) * rate *
                         eqm_stoichiometry(j, i) /
                         (std::pow(eqm_activity[j], promoting_indices[index]) +
                          std::pow(promoting_half_saturation[index], promoting_indices[index])) /
                         basis_molality[i];
      else
        drate_dmol[i] -= promoting_monod_indices[index] * promoting_indices[index] *
                         std::pow(eqm_molality[j], promoting_indices[index]) * rate *
                         eqm_stoichiometry(j, i) /
                         (std::pow(eqm_molality[j], promoting_indices[index]) +
                          std::pow(promoting_half_saturation[index], promoting_indices[index])) /
                         basis_molality[i];
    }
  }
  // derivative of the activity-product term
  Real deriv_ap_term = 0.0;
  if (theta_term != 1.0)
    deriv_ap_term =
        description.eta * rate / std::abs(1 - theta_term) * (-description.theta) * theta_term;
  else // theta_term = 1, so rate = 0 (unless eta = 0 too)
  {
    if (description.eta > 1)
      deriv_ap_term = 0.0;
    else if (description.eta == 1.0)
      deriv_ap_term = 0.0; // rate_no_theta_term * description.theta * theta_term;
    else if (description.eta == 0.0)
      deriv_ap_term = 0.0;
    else
      deriv_ap_term = std::numeric_limits<Real>::max();
  }
  if (theta_term > 1.0)
    deriv_ap_term = -deriv_ap_term;
  for (unsigned i = 1; i < num_basis; ++i)
    if (!(basis_species_gas[i] || kin_stoichiometry(kin, i) == 0.0 || deriv_ap_term == 0.0))
      drate_dmol[i] += deriv_ap_term * kin_stoichiometry(kin, i) / basis_molality[i];
}
}
