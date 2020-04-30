//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PertinentGeochemicalSystem.h"
#include "GeochemistryIonicStrength.h"

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
};

/**
 * Computes activity coefficients for non-minerals and non-gases (since these species do not have
 * activity coefficients). Also computes the activity of water.
 */
class GeochemistryActivityCoefficients
{
public:
  /// Method used by this class to compute activity coefficients and activity of water
  enum class ActivityCoefficientMethodEnum
  {
    DEBYE_HUCKEL
  };

  /**
   * @param method Method used by this class to compute activity coefficients and activity of water
   * @param is_calculator Calculates ionic strengths
   */
  GeochemistryActivityCoefficients(ActivityCoefficientMethodEnum method,
                                   const GeochemistryIonicStrength & is_calculator);

  /**
   * Set internal parameters, such as the ionic strength and Debye-Huckel parameters, prior to
   * computing activity coefficients and activity of water.  Ensure the ionic strength calculator
   * (eg, maxIonicStrength) is set appropriately before calling this function.
   * @param temperature the temperature in degC
   * @param mgd the Model Geochemical database
   * @param basis_species_molality Molalities of the basis species in mgd
   * @param eqm_species_molality Molalities of the equilibrium species in mgd
   * @param kin_species_molality Molalities of the kinetic species
   */
  void setInternalParameters(Real temperature,
                             const ModelGeochemicalDatabase & mgd,
                             const std::vector<Real> & basis_species_molality,
                             const std::vector<Real> & eqm_species_molality,
                             const std::vector<Real> & kin_species_molality);

  /**
   * Computes and returns the activity of water.  Note that you will probably want to call
   * setInternalParameters prior to calling this method
   * @return the activity of water
   */
  Real waterActivity() const;

  /**
   * Compute the activity coefficients and store them in basis_activity_coef and eqm_activity_coef
   * Note:
   * - you will probably want to call setInternalParameters prior to calling this method
   * - the activity coefficient for water (basis species = 0) is not computed since it is
   * meaningless: use getWaterActivity() instead
   * - the activity coefficient for any mineral is not computed since minerals do not have activity
   * coefficients
   * - the activity coefficient for any gas is not computed since gases do not have activity
   * coefficients
   *  Hence, the elements in basis_activity_coef and eqm_activity_coef corresponding to
   * these species will be undefined after this method returns
   */
  void buildActivityCoefficients(const ModelGeochemicalDatabase & mgd,
                                 std::vector<Real> & basis_activity_coef,
                                 std::vector<Real> & eqm_activity_coef) const;

  /**
   * @return the Debye-Huckel parameters
   */
  const DebyeHuckelParameters & getDebyeHuckel() const;

  /// Return the current value of ionic strength
  Real getIonicStrength() const;

  /// Return the current value of stoichiometric ionic strength
  Real getStoichiometricIonicStrength() const;

private:
  /// method used by this object to compute activity coefficients
  const ActivityCoefficientMethodEnum _method;

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
};
