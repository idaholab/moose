//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryActivityCoefficientsDebyeHuckel.h"
#include "GeochemistryActivityCalculators.h"

GeochemistryActivityCoefficientsDebyeHuckel::GeochemistryActivityCoefficientsDebyeHuckel(
    const GeochemistryIonicStrength & is_calculator)
  : GeochemistryActivityCoefficients(),
    _is_calculator(is_calculator),
    _ionic_strength(1.0),
    _sqrt_ionic_strength(1.0),
    _stoichiometric_ionic_strength(1.0),
    _num_basis(0),
    _num_eqm(0),
    _dh()
{
}

void
GeochemistryActivityCoefficientsDebyeHuckel::setInternalParameters(
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
  _dh.A = 0.5092;
  _dh.B = 0.3283;
  _dh.Bdot = 0.035;
  // water Debye-Huckel at 25degC
  _dh.a_water = 1.45397;
  _dh.b_water = 0.022357;
  _dh.c_water = 0.0093804;
  _dh.d_water = -0.0005262;
  // neutral species Debye-Huckel at 25degC
  _dh.a_neutral = 0.1127;
  _dh.b_neutral = -0.01049;
  _dh.c_neutral = 0.001545;
  _dh.d_neutral = 0.0;
}

const DebyeHuckelParameters &
GeochemistryActivityCoefficientsDebyeHuckel::getDebyeHuckel() const
{
  return _dh;
}

Real
GeochemistryActivityCoefficientsDebyeHuckel::waterActivity() const
{
  return std::exp(GeochemistryActivityCalculators::lnActivityDHBdotWater(
      _stoichiometric_ionic_strength, _dh.A, _dh.a_water, _dh.b_water, _dh.c_water, _dh.d_water));
}

void
GeochemistryActivityCoefficientsDebyeHuckel::buildActivityCoefficients(
    const ModelGeochemicalDatabase & mgd,
    std::vector<Real> & basis_activity_coef,
    std::vector<Real> & eqm_activity_coef) const
{
  basis_activity_coef.resize(_num_basis);
  eqm_activity_coef.resize(_num_eqm);

  for (unsigned basis_i = 1; basis_i < _num_basis; ++basis_i) // don't loop over water
    if (mgd.basis_species_mineral[basis_i] || mgd.basis_species_gas[basis_i])
      continue; // these should never be used
    else if (mgd.basis_species_radius[basis_i] == -0.5)
    {
      basis_activity_coef[basis_i] = std::pow(
          10.0,
          GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(
              _ionic_strength, _dh.a_neutral, _dh.b_neutral, _dh.c_neutral, _dh.d_neutral));
    }
    else if (mgd.basis_species_radius[basis_i] == -1.0)
    {
      basis_activity_coef[basis_i] =
          std::pow(10.0,
                   GeochemistryActivityCalculators::log10ActCoeffDHBdotAlternative(_ionic_strength,
                                                                                   _dh.Bdot));
    }
    else if (mgd.basis_species_radius[basis_i] == -1.5)
    {
      basis_activity_coef[basis_i] = 1.0;
    }
    else if (mgd.basis_species_charge[basis_i] == 0.0)
    {
      basis_activity_coef[basis_i] = 1.0;
    }
    else
    {
      basis_activity_coef[basis_i] = std::pow(
          10.0,
          GeochemistryActivityCalculators::log10ActCoeffDHBdot(mgd.basis_species_charge[basis_i],
                                                               mgd.basis_species_radius[basis_i],
                                                               _sqrt_ionic_strength,
                                                               _dh.A,
                                                               _dh.B,
                                                               _dh.Bdot));
    }

  for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
    if (mgd.eqm_species_mineral[eqm_j] || mgd.eqm_species_gas[eqm_j])
      continue;
    else if (mgd.eqm_species_radius[eqm_j] == -0.5)
    {
      eqm_activity_coef[eqm_j] = std::pow(
          10.0,
          GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(
              _ionic_strength, _dh.a_neutral, _dh.b_neutral, _dh.c_neutral, _dh.d_neutral));
    }
    else if (mgd.eqm_species_radius[eqm_j] == -1.0)
    {
      eqm_activity_coef[eqm_j] =
          std::pow(10.0,
                   GeochemistryActivityCalculators::log10ActCoeffDHBdotAlternative(_ionic_strength,
                                                                                   _dh.Bdot));
    }
    else if (mgd.eqm_species_radius[eqm_j] == -1.5)
    {
      eqm_activity_coef[eqm_j] = 1.0;
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
                                                               _dh.A,
                                                               _dh.B,
                                                               _dh.Bdot));
    }
}

Real
GeochemistryActivityCoefficientsDebyeHuckel::getIonicStrength() const
{
  return _ionic_strength;
}

Real
GeochemistryActivityCoefficientsDebyeHuckel::getStoichiometricIonicStrength() const
{
  return _stoichiometric_ionic_strength;
}
