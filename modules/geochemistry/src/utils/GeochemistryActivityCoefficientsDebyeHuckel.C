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
    const GeochemistryIonicStrength & is_calculator, const GeochemicalDatabaseReader & db)
  : GeochemistryActivityCoefficients(),
    _numT(db.getTemperatures().size()),
    _database_dh_params(db.getDebyeHuckel()),
    _database_dh_water((db.getNeutralSpeciesActivity().count("h2o") == 1)
                           ? db.getNeutralSpeciesActivity().at("h2o")
                           : GeochemistryNeutralSpeciesActivity()),
    _database_dh_neutral((db.getNeutralSpeciesActivity().count("co2") == 1)
                             ? db.getNeutralSpeciesActivity().at("co2")
                             : GeochemistryNeutralSpeciesActivity()),
    _is_calculator(is_calculator),
    _ionic_strength(1.0),
    _sqrt_ionic_strength(1.0),
    _stoichiometric_ionic_strength(1.0),
    _num_basis(0),
    _num_eqm(0),
    _dh(),
    _interp_A(db.getTemperatures(),
              (_database_dh_params.adh.size() == _numT) ? _database_dh_params.adh
                                                        : std::vector<Real>(_numT, 0.0),
              db.getLogKModel()),
    _interp_B(db.getTemperatures(),
              (_database_dh_params.bdh.size() == _numT) ? _database_dh_params.bdh
                                                        : std::vector<Real>(_numT, 0.0),
              db.getLogKModel()),
    _interp_Bdot(db.getTemperatures(),
                 (_database_dh_params.bdot.size() == _numT) ? _database_dh_params.bdot
                                                            : std::vector<Real>(_numT, 0.0),
                 db.getLogKModel()),
    _interp_a_water(db.getTemperatures(),
                    (_database_dh_water.a.size() == _numT) ? _database_dh_water.a
                                                           : std::vector<Real>(_numT, 0.0),
                    db.getLogKModel()),
    _interp_b_water(db.getTemperatures(),
                    (_database_dh_water.b.size() == _numT) ? _database_dh_water.b
                                                           : std::vector<Real>(_numT, 0.0),
                    db.getLogKModel()),
    _interp_c_water(db.getTemperatures(),
                    (_database_dh_water.c.size() == _numT) ? _database_dh_water.c
                                                           : std::vector<Real>(_numT, 0.0),
                    db.getLogKModel()),
    _interp_d_water(db.getTemperatures(),
                    (_database_dh_water.d.size() == _numT) ? _database_dh_water.d
                                                           : std::vector<Real>(_numT, 0.0),
                    db.getLogKModel()),
    _interp_a_neutral(db.getTemperatures(),
                      (_database_dh_neutral.a.size() == _numT) ? _database_dh_neutral.a
                                                               : std::vector<Real>(_numT, 0.0),
                      db.getLogKModel()),
    _interp_b_neutral(db.getTemperatures(),
                      (_database_dh_neutral.b.size() == _numT) ? _database_dh_neutral.b
                                                               : std::vector<Real>(_numT, 0.0),
                      db.getLogKModel()),
    _interp_c_neutral(db.getTemperatures(),
                      (_database_dh_neutral.c.size() == _numT) ? _database_dh_neutral.c
                                                               : std::vector<Real>(_numT, 0.0),
                      db.getLogKModel()),
    _interp_d_neutral(db.getTemperatures(),
                      (_database_dh_neutral.d.size() == _numT) ? _database_dh_neutral.d
                                                               : std::vector<Real>(_numT, 0.0),
                      db.getLogKModel())
{
  _interp_A.generate();
  _interp_B.generate();
  _interp_Bdot.generate();
  _interp_a_water.generate();
  _interp_b_water.generate();
  _interp_c_water.generate();
  _interp_d_water.generate();
  _interp_a_neutral.generate();
  _interp_b_neutral.generate();
  _interp_c_neutral.generate();
  _interp_d_neutral.generate();
}

void
GeochemistryActivityCoefficientsDebyeHuckel::setInternalParameters(
    Real temperature,
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
  // Debye-Huckel base parameters
  _dh.A = _interp_A.sample(temperature);
  _dh.B = _interp_B.sample(temperature);
  _dh.Bdot = _interp_Bdot.sample(temperature);
  // water Debye-Huckel
  _dh.a_water = _interp_a_water.sample(temperature);
  _dh.b_water = _interp_b_water.sample(temperature);
  _dh.c_water = _interp_c_water.sample(temperature);
  _dh.d_water = _interp_d_water.sample(temperature);
  // neutral species Debye-Huckel
  _dh.a_neutral = _interp_a_neutral.sample(temperature);
  _dh.b_neutral = _interp_b_neutral.sample(temperature);
  _dh.c_neutral = _interp_c_neutral.sample(temperature);
  _dh.d_neutral = _interp_d_neutral.sample(temperature);
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
