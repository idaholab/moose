//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

class AddCoupledSolidKinSpeciesAction : public Action
{
public:
  static InputParameters validParams();

  AddCoupledSolidKinSpeciesAction(const InputParameters & params);

  virtual void act() override;

private:
  /// Basis set of primary species
  const std::vector<NonlinearVariableName> _primary_species;
  /// Secondary species added as AuxVariables
  const std::vector<AuxVariableName> _secondary_species;
  /// Secondary solid species read by the parser
  std::vector<VariableName> _solid_kinetic_species;
  /// Primary species involved in the ith kinetic reaction
  std::vector<std::vector<VariableName>> _primary_species_involved;
  /// Secondary solid species involved the ith primary species
  std::vector<std::vector<VariableName>> _kinetic_species_involved;
  /// Stoichiometric coefficients for each primary species in each reaction
  std::vector<std::vector<Real>> _stos;
  /// Weight of each primary species in each reaction
  std::vector<std::vector<Real>> _weights;
  /// Reaction network read from input file
  std::string _input_reactions;
  /// Vector of parsed reactions
  std::vector<std::string> _reactions;
  /// Number of reactions
  unsigned int _num_reactions;
  /// Log10 of equilibrium constant
  const std::vector<Real> _logk;
  /// Specific reactive surface area, m^2/L solution
  const std::vector<Real> _r_area;
  /// Reference kinetic rate constant
  const std::vector<Real> _ref_kconst;
  /// Activation energy
  const std::vector<Real> _e_act;
  /// Gas constant, (Default 8.314 J/mol/K)
  const Real _gas_const;
  /// Reference temperature
  const std::vector<Real> _ref_temp;
  /// Actual system temperature
  const std::vector<VariableName> _sys_temp;
};
