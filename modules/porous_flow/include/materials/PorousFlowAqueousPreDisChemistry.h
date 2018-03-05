//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWMASSFRACTIONAQUEOUSPREDISCHEMISTRY_H
#define POROUSFLOWMASSFRACTIONAQUEOUSPREDISCHEMISTRY_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowAqueousPreDisChemistry;

template <>
InputParameters validParams<PorousFlowAqueousPreDisChemistry>();

/**
 * Material designed to form a std::vector
 * of mass fractions of mineral concentrations from primary-species concentrations
 * for an equilibrium precipitation-dissolution chemistry reaction system
 */
class PorousFlowAqueousPreDisChemistry : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowAqueousPreDisChemistry(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;
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
  virtual void computeQpReactionRates();

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
   * Computes derivative of the reaction rate with respect to the primary concentrations
   * @param reaction_num The reaction number corresponding to the secondary-species concentration
   * @param drr drr[i] = d(reactionRate[reaction_num])/d(primary_species[i])
   */
  virtual void dQpReactionRate_dprimary(unsigned reaction_num, std::vector<Real> & drr) const;

  /**
   * Computes derivative of the reaction rate with respect to the temperature
   * @param reaction_num The reaction number corresponding to the secondary-species concentration
   */
  virtual Real dQpReactionRate_dT(unsigned reaction_num) const;

  Real rateConstantQp(unsigned reaction_num) const;

  /// old values of the porosity
  const MaterialProperty<Real> & _porosity_old;

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

  // old values of the mineral species concentrations
  const MaterialProperty<std::vector<Real>> & _sec_conc_old;

  // mineral saturation ratio - a useful temporary variable during computeQpProperties
  std::vector<Real> _mineral_sat;

  // whether the reaction rate has to be bounded in order that the precipitate stays inside [0, 1]
  std::vector<bool> _bounded_rate;

  // reaction rate of mineralisation
  MaterialProperty<std::vector<Real>> & _reaction_rate;

  // d(reaction rate of mineralisation)/d(porous flow var)
  MaterialProperty<std::vector<std::vector<Real>>> & _dreaction_rate_dvar;

  // reactive surface area (m^2/L) for each reaction
  const std::vector<Real> _r_area;

  // molar volume (L/mol) for each secondary species
  const std::vector<Real> _molar_volume;

  // rate constant (mol/(m^2 s)) at reference temperature for each reaction
  const std::vector<Real> _ref_kconst;

  // activation energy (J/mol) for each reaction
  const std::vector<Real> _e_act;

  // gas constant (J/(mol K))
  const Real _gas_const;

  // 1/reference_temperature (1/K)
  const Real _one_over_ref_temp;

  // theta exponent for the precipitation-dissolution for each reaction
  const std::vector<Real> _theta_exponent;

  // eta exponent for the precipitation-dissolution for each reaction
  const std::vector<Real> _eta_exponent;

  // initial values of the secondary species concentrations
  std::vector<const VariableValue *> _initial_conc;
};

#endif // POROUSFLOWMASSFRACTIONAQUEOUSPREDISCHEMISTRY_H
