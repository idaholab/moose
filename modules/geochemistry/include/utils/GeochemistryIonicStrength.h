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
 * Calculators to compute ionic strength and stoichiometric ionic strength
 */
class GeochemistryIonicStrength
{
public:
  /**
   * @param max_ionic_strength maximum value of ionic strength ever returned by this class
   * @param max_stoichiometric_ionic_strength maximum value of stoichiometric ionic strength ever
   * returned by this class
   * @param use_only_basis_molality If true, use only the basis molalities in the ionic strength and
   * stoichiometric ionic strength calculations.  Since basis molality is usually much greater than
   * equilibrium molality, and the whole concept of activity coefficients depending on ionic
   * strength is only approximate in practice, setting use_only_basis_molality=true is often a
   * reasonable approximation, and it can help with convergence of geochemistry algorithms since it
   * eliminates problems associated with unphysical huge equilibrium molalities that can occur
   * during Newton-iteration to the equilibrium solution
   * @param use_only_Cl_molality If true, set the stoichiometric ionic strength to the Chlorine-
   * molality.  Note that this is the method used by the Geochemists Workbench
   */
  GeochemistryIonicStrength(Real max_ionic_strength,
                            Real max_stoichiometric_ionic_strength,
                            bool use_only_basis_molality,
                            bool use_only_Cl_molality);

  bool operator==(const GeochemistryIonicStrength & rhs) const
  {
    return (_max_ionic_strength == rhs._max_ionic_strength) &&
           (_max_stoichiometric_ionic_strength == rhs._max_stoichiometric_ionic_strength) &&
           (_use_only_basis_molality == rhs._use_only_basis_molality) &&
           (_use_only_Cl_molality == rhs._use_only_Cl_molality);
  };

  /**
   * Compute ionic strength
   * @param mgd the Model Geochemical database
   * @param basis_species_molality Molalities of the basis species in mgd
   * @param eqm_species_molality Molalities of the equilibrium species in mgd
   * @param kin_species_molality Molalities of the kinetic species
   * @return the ionic strength of the aqueous solution
   */
  Real ionicStrength(const ModelGeochemicalDatabase & mgd,
                     const std::vector<Real> & basis_species_molality,
                     const std::vector<Real> & eqm_species_molality,
                     const std::vector<Real> & kin_species_molality) const;

  /**
   * Compute stoichiometric ionic strength
   * @param mgd the Model Geochemical database
   * @param basis_species_molality Molalities of the basis species in mgd
   * @param eqm_species_molality Molalities of the equilibrium species in mgd
   * @param kin_species_molality Molalities of the kinetic species
   * @return the stoichiometric ionic strength of the aqueous solution
   */
  Real stoichiometricIonicStrength(const ModelGeochemicalDatabase & mgd,
                                   const std::vector<Real> & basis_species_molality,
                                   const std::vector<Real> & eqm_species_molality,
                                   const std::vector<Real> & kin_species_molality) const;

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

  /// Set the value of use_only_Cl_molality
  void setUseOnlyClMolality(bool use_only_Cl_molality);

  /// Return the value of use_only_Cl_molality
  Real getUseOnlyClMolality() const;

private:
  /// maximum ionic strength
  Real _max_ionic_strength;

  /// maximum stoichiometric ionic strength
  Real _max_stoichiometric_ionic_strength;

  /// use only basis molality in the ionic strength calculations
  bool _use_only_basis_molality;

  /// set the stoichiometric ionic strength to the Cl- molality
  bool _use_only_Cl_molality;
};
