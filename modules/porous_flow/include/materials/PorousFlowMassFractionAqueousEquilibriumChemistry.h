//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWMASSFRACTIONAQUEOUSCHEMISTRYCHEMISTRY_H
#define POROUSFLOWMASSFRACTIONAQUEOUSCHEMISTRYCHEMISTRY_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowMassFractionAqueousEquilibriumChemistry;

template <>
InputParameters validParams<PorousFlowMassFractionAqueousEquilibriumChemistry>();

/**
 * Material designed to form a std::vector<std::vector>
 * of mass fractions from primary-species concentrations
 * and secondary-species concentrations
 * for an equilibrium aqueous chemistry reaction system
 */
class PorousFlowMassFractionAqueousEquilibriumChemistry : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowMassFractionAqueousEquilibriumChemistry(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * The stoichiometric coefficient
   * @param reaction_num Reaction number (0, ..., _num_reactions - 1)
   * @param primary_num The number of the primary species (0, ..., _num_primary - 1)
   */
  Real stoichiometry(unsigned reaction_num, unsigned primary_num) const;

  /**
   * Compute the secondary-species concentration as defined by the chemistry
   * Must be overridden by derived classes
   */
  virtual void computeQpSecondaryConcentrations();

  /**
   * Initialises (at _t_step = 0) the secondary concentrations
   */
  virtual void initQpSecondaryConcentrations();

  /**
   * Checks gamp[i] = _primary_activity_coefficients[i] * (*_primary[i])[qp].
   * Returns:
   *   if all of these are positive, then zero_count = 0, zero_conc_index = 0
   *   if one of these is zero, then zero_count = 1, zero_conc_index = the index of the zero gamp
   *   if more than one is zero, then zero_count = 2, and zero_conc_index is the index of the 2nd
   * zero
   */
  void findZeroConcentration(unsigned & zero_conc_index, unsigned & zero_count) const;

  /**
   * Computes derivative of the secondary concentration with respect to the primary concentrations
   * Must be overridden by derived classes
   * @param reaction_num The reaction number corresponding to the secondary-species concentration
   * @param dsc dsc[i] = d(secondaryConcentration[reaction_num])/d(primary_species[i])
   */
  virtual void dQpSecondaryConcentration_dprimary(unsigned reaction_num,
                                                  std::vector<Real> & dsc) const;

  /**
   * Computes derivative of the secondary concentration with respect to the temperature
   * Must be overridden by derived classes
   * @param reaction_num The reaction number corresponding to the secondary-species concentration
   * @param dsc dsc[i] = d(secondaryConcentration[reaction_num])/d(primary_species[i])
   */
  virtual Real dQpSecondaryConcentration_dT(unsigned reaction_num) const;

  /// Mass fraction matrix at quadpoint or nodes
  MaterialProperty<std::vector<std::vector<Real>>> & _mass_frac;

  /// Gradient of the mass fraction matrix at the quad points
  MaterialProperty<std::vector<std::vector<RealGradient>>> * const _grad_mass_frac;

  /// Derivative of the mass fraction matrix with respect to the porous flow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;

  /// Secondary concentrations at quadpoint or nodes
  MaterialProperty<std::vector<Real>> & _sec_conc;

  /// Derivative of the secondary concentrations with respect to the porous flow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dsec_conc_dvar;

  /// temperature
  const MaterialProperty<Real> & _temperature;

  /// d(temperature)/(d porflow variable)
  const MaterialProperty<std::vector<Real>> & _dtemperature_dvar;

  /// Number of primary species
  const unsigned int _num_primary;

  /// number of equations in the aqueous geochemistry system
  const unsigned int _num_reactions;

  /// equilibrium constants (dimensionless)
  const std::vector<Real> _equilibrium_constants;

  /// activity coefficients for the primary species (dimensionless)
  const std::vector<Real> _primary_activity_coefficients;

  /// stoichiometry defining the aqeuous geochemistry equilibrium reactions
  const std::vector<Real> _reactions;

  /// the variable number of the primary variables
  std::vector<unsigned int> _primary_var_num;

  /// Values of the primary species' concentrations (dimensionless)
  std::vector<const VariableValue *> _primary;

  /// the gradient of the primary variables (units = m^-1)
  std::vector<const VariableGradient *> _grad_primary;

  /// activity coefficients for the secondary species
  const std::vector<Real> _secondary_activity_coefficients;
};

#endif // POROUSFLOWMASSFRACTIONAQUEOUSCHEMISTRYCHEMISTRY_H
