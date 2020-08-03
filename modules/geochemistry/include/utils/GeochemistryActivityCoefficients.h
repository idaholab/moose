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

/**
 * Base class to compute activity coefficients for non-minerals and non-gases (since these species
 * do not have activity coefficients). Also computes the activity of water.
 */
class GeochemistryActivityCoefficients
{
public:
  GeochemistryActivityCoefficients(){};

  bool operator==(const GeochemistryActivityCoefficients & /*rhs*/) const { return true; };

  /**
   * Sets internal parameters, such as the ionic strength and Debye-Huckel parameters, prior to
   * computing activity coefficients and activity of water.  If using a Debye-Huckel activity model,
   * you must ensure the ionic strength calculator (eg, maxIonicStrength) is set appropriately
   * before calling this function.
   * @param temperature the temperature in degC
   * @param mgd the Model Geochemical database
   * @param basis_species_molality Molalities of the basis species in mgd
   * @param eqm_species_molality Molalities of the equilibrium species in mgd
   * @param kin_species_molality Molalities of the kinetic species
   */
  virtual void setInternalParameters(Real temperature,
                                     const ModelGeochemicalDatabase & mgd,
                                     const std::vector<Real> & basis_species_molality,
                                     const std::vector<Real> & eqm_species_molality,
                                     const std::vector<Real> & kin_species_molality) = 0;

  /**
   * Computes and returns the activity of water.  Note that you will probably want to call
   * setInternalParameters prior to calling this method
   * @return the activity of water
   */
  virtual Real waterActivity() const = 0;

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
  virtual void buildActivityCoefficients(const ModelGeochemicalDatabase & mgd,
                                         std::vector<Real> & basis_activity_coef,
                                         std::vector<Real> & eqm_activity_coef) const = 0;
};
