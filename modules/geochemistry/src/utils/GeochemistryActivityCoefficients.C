//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryActivityCoefficients.h"
#include "GeochemistryActivityCalculators.h"

GeochemistryActivityCoefficients::GeochemistryActivityCoefficients(
    ActivityCoefficientMethodEnum method,
    Real max_ionic_strength,
    Real max_stoichiometric_ionic_strength,
    bool use_only_basis_molality)
  : _method(method),
    _is_calculator(max_ionic_strength, max_stoichiometric_ionic_strength, use_only_basis_molality),
    _ionic_strength(1.0),
    _sqrt_ionic_strength(1.0),
    _stoichiometric_ionic_strength(1.0),
    _num_basis(0),
    _num_eqm(0),
    _dhA(0.0),
    _dhB(0.0),
    _dhBdot(0.0),
    _dha(0.0),
    _dhb(0.0),
    _dhc(0.0),
    _dhd(0.0),
    _dhatilde(0.0),
    _dhbtilde(0.0),
    _dhctilde(0.0),
    _dhdtilde(0.0)
{
}

void
GeochemistryActivityCoefficients::setInternalParameters(
    Real /*temperature*/,
    const ModelGeochemicalDatabase & mgd,
    const std::vector<Real> & basis_species_molality,
    const std::vector<Real> & eqm_species_molality,
    const std::vector<Real> & kin_species_molality)
{
  _ionic_strength = _is_calculator.ionicStrength(
      mgd, basis_species_molality, eqm_species_molality, kin_species_molality);
  _sqrt_ionic_strength = std::sqrt(_ionic_strength);
  _stoichiometric_ionic_strength = _is_calculator.stoichiometricIonicStrength(
      mgd, basis_species_molality, eqm_species_molality, kin_species_molality);
  _num_basis = mgd.basis_species_index.size();
  _num_eqm = mgd.eqm_species_index.size();
  // when temperature least-squares is completed the following lines will be changed
  // charged species Debye-Huckel at 25degC
  _dhA = 0.5092;
  _dhB = 0.3283;
  _dhBdot = 0.035;
  // water Debye-Huckel at 25degC
  _dhatilde = 1.45397;
  _dhbtilde = 0.022357;
  _dhctilde = 0.0093804;
  _dhdtilde = -0.0005262;
  // neutral species Debye-Huckel at 25degC
  _dha = 0.1127;
  _dhb = -0.01049;
  _dhc = 0.001545;
  _dhd = 0.0;
}

std::vector<Real>
GeochemistryActivityCoefficients::getDebyeHuckel() const
{
  return {_dhA, _dhB, _dhBdot, _dhatilde, _dhbtilde, _dhctilde, _dhdtilde, _dha, _dhb, _dhc, _dhd};
}

Real
GeochemistryActivityCoefficients::waterActivity() const
{
  switch (_method)
  {
    case ActivityCoefficientMethodEnum::DEBYE_HUCKEL:
      return std::exp(GeochemistryActivityCalculators::lnActivityDHBdotWater(
          _stoichiometric_ionic_strength, _dhA, _dhatilde, _dhbtilde, _dhctilde, _dhdtilde));
  }
}

void
GeochemistryActivityCoefficients::buildActivityCoefficients(
    const ModelGeochemicalDatabase & mgd,
    std::vector<Real> & basis_activity_coef,
    std::vector<Real> & eqm_activity_coef) const
{
  basis_activity_coef.resize(_num_basis);
  eqm_activity_coef.resize(_num_eqm);

  switch (_method)
  {
    case ActivityCoefficientMethodEnum::DEBYE_HUCKEL:
    {
      for (unsigned basis_i = 1; basis_i < _num_basis; ++basis_i) // don't loop over water
        if (mgd.basis_species_mineral[basis_i] || mgd.basis_species_gas[basis_i])
          continue; // these should never be used
        else if (mgd.basis_species_radius[basis_i] == -0.5)
        {
          basis_activity_coef[basis_i] =
              std::pow(10.0,
                       GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(
                           _ionic_strength, _dha, _dhb, _dhc, _dhd));
        }
        else if (mgd.basis_species_radius[basis_i] == -1.0)
        {
          basis_activity_coef[basis_i] =
              std::pow(10.0,
                       GeochemistryActivityCalculators::log10ActCoeffDHBdotAlternative(
                           _ionic_strength, _dhBdot));
        }
        else if (mgd.basis_species_charge[basis_i] == 0.0)
        {
          basis_activity_coef[basis_i] = 1.0;
        }
        else
        {
          basis_activity_coef[basis_i] =
              std::pow(10.0,
                       GeochemistryActivityCalculators::log10ActCoeffDHBdot(
                           mgd.basis_species_charge[basis_i],
                           mgd.basis_species_radius[basis_i],
                           _sqrt_ionic_strength,
                           _dhA,
                           _dhB,
                           _dhBdot));
        }

      for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
        if (mgd.eqm_species_mineral[eqm_j] || mgd.eqm_species_gas[eqm_j])
          continue;
        else if (mgd.eqm_species_radius[eqm_j] == -0.5)
        {
          eqm_activity_coef[eqm_j] =
              std::pow(10.0,
                       GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(
                           _ionic_strength, _dha, _dhb, _dhc, _dhd));
        }
        else if (mgd.eqm_species_radius[eqm_j] == -1.0)
        {
          eqm_activity_coef[eqm_j] =
              std::pow(10.0,
                       GeochemistryActivityCalculators::log10ActCoeffDHBdotAlternative(
                           _ionic_strength, _dhBdot));
        }
        else if (mgd.eqm_species_charge[eqm_j] == 0.0)
        {
          eqm_activity_coef[eqm_j] = 1.0;
        }
        else
        {
          eqm_activity_coef[eqm_j] = std::pow(
              10.0,
              GeochemistryActivityCalculators::log10ActCoeffDHBdot(mgd.eqm_species_charge[eqm_j],
                                                                   mgd.eqm_species_radius[eqm_j],
                                                                   _sqrt_ionic_strength,
                                                                   _dhA,
                                                                   _dhB,
                                                                   _dhBdot));
        }
      break;
    }
  }
}

Real
GeochemistryActivityCoefficients::getIonicStrength() const
{
  return _ionic_strength;
}

Real
GeochemistryActivityCoefficients::getStoichiometricIonicStrength() const
{
  return _stoichiometric_ionic_strength;
}

void
GeochemistryActivityCoefficients::setMaxIonicStrength(Real max_ionic_strength)
{
  _is_calculator.setMaxIonicStrength(max_ionic_strength);
}

Real
GeochemistryActivityCoefficients::getMaxIonicStrength() const
{
  return _is_calculator.getMaxIonicStrength();
}

void
GeochemistryActivityCoefficients::setMaxStoichiometricIonicStrength(
    Real max_stoichiometric_ionic_strength)
{
  _is_calculator.setMaxStoichiometricIonicStrength(max_stoichiometric_ionic_strength);
}

Real
GeochemistryActivityCoefficients::getMaxStoichiometricIonicStrength() const
{
  return _is_calculator.getMaxStoichiometricIonicStrength();
}

void
GeochemistryActivityCoefficients::setUseOnlyBasisMolality(bool use_only_basis_molality)
{
  _is_calculator.setUseOnlyBasisMolality(use_only_basis_molality);
}

Real
GeochemistryActivityCoefficients::getUseOnlyBasisMolality() const
{
  return _is_calculator.getUseOnlyBasisMolality();
}
