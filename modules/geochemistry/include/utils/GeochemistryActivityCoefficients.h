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

/**
 * Computes activity coefficients for non-minerals and non-gases (since these are meaningless). Also
 * computes the activity of water.
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
   * @param max_ionic_strength maximum value of ionic strength (used if method=DEBYE_HUCKEL)
   * @param max_stoichiometric_ionic_strength maximum value of stoichiometric ionic strength (used
   * if method=DEBYE_HUCKEL)
   * @param use_only_basis_molality If true, use only the basis molalities in the ionic strength and
   * stoichiometric ionic strength calculations (used if method=DEBYE_HUCKEL)
   */
  GeochemistryActivityCoefficients(ActivityCoefficientMethodEnum method,
                                   Real max_ionic_strength,
                                   Real max_stoichiometric_ionic_strength,
                                   bool use_only_basis_molality);

  /**
   * Set internal parameters, such as the ionic strength and Debye-Huckel parameters, prior to
   * computing activity coefficients and activity of water.
   * @param temperature the temperature in degC
   * @param mgd the Model Geochemical database
   * @param basis_species_molality Molalities of the basis species in mgd
   * @param eqm_species_molality Molalities of the equilibrium species in mgd
   * @param kin_species_molality Molalities of the kinetic species
   * @return the ionic strength of the aqueous solution
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
   * meaningless: get getWaterActivity() instead
   * - the activity coefficient for any mineral is not computed since it is meaningless
   * - the activity coefficient for any gas is not computed since it is meaningless
   * Hence, the elements in basis_activity_coef and eqm_activity_coef corresponding to these
   * species will be undefined after this method returns
   */
  void buildActivityCoefficients(const ModelGeochemicalDatabase & mgd,
                                 std::vector<Real> & basis_activity_coef,
                                 std::vector<Real> & eqm_activity_coef) const;

  /**
   * @return the vector {A, B, Bdot, a(water), b(water), c(water), d(water), a(neutral), b(neutral),
   * c(neutral), d(neutral)} used in the DebyeHuckel model
   */
  std::vector<Real> getDebyeHuckel() const;

  /// Return the current value of ionic strength
  Real getIonicStrength() const;

  /// Return the current value of stoichiometric ionic strength
  Real getStoichiometricIonicStrength() const;

  /// Set the maximum ionic strength
  void setMaxIonicStrength(Real max_ionic_strength);

  /// Return the value of maximum ionic strength
  Real getMaxIonicStrength() const;

  /// Set the maximum stoichiometric ionic strength
  void setMaxStoichiometricIonicStrength(Real max_stoichiometric_ionic_strength);

  /// Return the value of maximum stoichiometric ionic strength
  Real getMaxStoichiometricIonicStrength() const;

  /// Set the value of use_only_basis_molality
  void setUseOnlyBasisMolality(bool use_only_basis_molality);

  /// Return the value of use_only_basis_molality
  Real getUseOnlyBasisMolality() const;

private:
  /// method used by this object to compute activity coefficients
  const ActivityCoefficientMethodEnum _method;

  /// ionic-strength calculator
  GeochemistryIonicStrength _is_calculator;

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

  /// Debye-Huckel parameter
  Real _dhA;
  /// Debye-Huckel parameter
  Real _dhB;
  /// Debye-Huckel parameter
  Real _dhBdot;
  /// Debye-Huckel parameter
  Real _dha;
  /// Debye-Huckel parameter
  Real _dhb;
  /// Debye-Huckel parameter
  Real _dhc;
  /// Debye-Huckel parameter
  Real _dhd;
  /// Debye-Huckel parameter
  Real _dhatilde;
  /// Debye-Huckel parameter
  Real _dhbtilde;
  /// Debye-Huckel parameter
  Real _dhctilde;
  /// Debye-Huckel parameter
  Real _dhdtilde;
};
