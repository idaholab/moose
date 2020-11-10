//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemistryActivityCoefficients.h"
#include "GeochemistryIonicStrength.h"
#include "EquilibriumConstantInterpolator.h"

struct DebyeHuckelParameters
{
  Real A;
  Real B;
  Real Bdot;
  Real a_water;
  Real b_water;
  Real c_water;
  Real d_water;
  Real a_neutral;
  Real b_neutral;
  Real c_neutral;
  Real d_neutral;

  DebyeHuckelParameters()
    : A(0.5092),
      B(0.3283),
      Bdot(0.035),
      a_water(1.45397),
      b_water(0.022357),
      c_water(0.0093804),
      d_water(-0.0005262),
      a_neutral(0.1127),
      b_neutral(-0.01049),
      c_neutral(0.001545),
      d_neutral(0.0)
  {
  }

  bool operator==(const DebyeHuckelParameters & rhs) const
  {
    return (A == rhs.A) && (B == rhs.B) && (Bdot == rhs.Bdot) && (a_water == rhs.a_water) &&
           (b_water == rhs.b_water) && (c_water == rhs.c_water) && (d_water == rhs.d_water) &&
           (a_neutral == rhs.a_neutral) && (b_neutral == rhs.b_neutral) &&
           (c_neutral == rhs.c_neutral) && (d_neutral == rhs.d_neutral);
  };
};

/**
 * Computes activity coefficients for non-minerals and non-gases (since these species do not have
 * activity coefficients). Also computes the activity of water.  Uses a Debye-Huckel model
 */
class GeochemistryActivityCoefficientsDebyeHuckel : public GeochemistryActivityCoefficients
{
public:
  /**
   * @param method Method used by this class to compute activity coefficients and activity of water
   * @param is_calculator Calculates ionic strengths
   * @param db Original geochemistry database: used to find the temperature interpolation type,
   * Debye-Huckel params, etc
   */
  GeochemistryActivityCoefficientsDebyeHuckel(const GeochemistryIonicStrength & is_calculator,
                                              const GeochemicalDatabaseReader & db);

  void setInternalParameters(Real temperature,
                             const ModelGeochemicalDatabase & mgd,
                             const std::vector<Real> & basis_species_molality,
                             const std::vector<Real> & eqm_species_molality,
                             const std::vector<Real> & kin_species_molality) override;

  Real waterActivity() const override;

  void buildActivityCoefficients(const ModelGeochemicalDatabase & mgd,
                                 std::vector<Real> & basis_activity_coef,
                                 std::vector<Real> & eqm_activity_coef) const override;

  /**
   * @return the Debye-Huckel parameters
   */
  const DebyeHuckelParameters & getDebyeHuckel() const;

  /// Return the current value of ionic strength
  Real getIonicStrength() const;

  /// Return the current value of stoichiometric ionic strength
  Real getStoichiometricIonicStrength() const;

private:
  /// number of temperature points in the database file
  const unsigned _numT;

  /// Debye-Huckel parameters found in the database
  const GeochemistryDebyeHuckel _database_dh_params;

  /// Debye-Huckel parameters found in the database for computing the water activities
  const GeochemistryNeutralSpeciesActivity _database_dh_water;

  /// Debye-Huckel parameters found in the database for computing the neutral (CO2) activities
  const GeochemistryNeutralSpeciesActivity _database_dh_neutral;

  /// ionic-strength calculator
  const GeochemistryIonicStrength & _is_calculator;

  /// current value of ionic strength
  Real _ionic_strength;

  /// current value of sqrt(ionic strength)
  Real _sqrt_ionic_strength;

  /// current value of stoichiometric ionic strength
  Real _stoichiometric_ionic_strength;

  /// number of basis species
  unsigned _num_basis;

  /// number of equilibrium species
  unsigned _num_eqm;

  /// Debye-Huckel parameters
  DebyeHuckelParameters _dh;

  /// Interpolator object for the Debye-Huckel parameter A
  EquilibriumConstantInterpolator _interp_A;

  /// Interpolator object for the Debye-Huckel parameter B
  EquilibriumConstantInterpolator _interp_B;

  /// Interpolator object for the Debye-Huckel parameter Bdot
  EquilibriumConstantInterpolator _interp_Bdot;

  /// Interpolator object for the Debye-Huckel parameter a_water
  EquilibriumConstantInterpolator _interp_a_water;

  /// Interpolator object for the Debye-Huckel parameter b_water
  EquilibriumConstantInterpolator _interp_b_water;

  /// Interpolator object for the Debye-Huckel parameter c_water
  EquilibriumConstantInterpolator _interp_c_water;

  /// Interpolator object for the Debye-Huckel parameter d_water
  EquilibriumConstantInterpolator _interp_d_water;

  /// Interpolator object for the Debye-Huckel parameter a_neutral
  EquilibriumConstantInterpolator _interp_a_neutral;

  /// Interpolator object for the Debye-Huckel parameter b_neutral
  EquilibriumConstantInterpolator _interp_b_neutral;

  /// Interpolator object for the Debye-Huckel parameter c_neutral
  EquilibriumConstantInterpolator _interp_c_neutral;

  /// Interpolator object for the Debye-Huckel parameter d_neutral
  EquilibriumConstantInterpolator _interp_d_neutral;
};
