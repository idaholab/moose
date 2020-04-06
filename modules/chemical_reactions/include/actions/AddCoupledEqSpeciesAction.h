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

#include "libmesh/vector_value.h"

class AddCoupledEqSpeciesAction : public Action
{
public:
  static InputParameters validParams();

  AddCoupledEqSpeciesAction(const InputParameters & params);

  virtual void act() override;

protected:
  /// Basis set of primary species
  const std::vector<NonlinearVariableName> _primary_species;
  /// Secondary species added as AuxVariables
  const std::vector<AuxVariableName> _secondary_species;
  /// Stoichiometric coefficients for each primary species in each reaction
  std::vector<std::vector<Real>> _stos;
  /// Weight of each primary species in each reaction
  std::vector<std::vector<Real>> _weights;
  /// Equilibrium constants for each reaction
  std::vector<Real> _eq_const;
  /// Equilibrium species
  std::vector<VariableName> _eq_species;
  /// Set of auxillary species
  std::set<std::string> _aux_species;
  /// Participation of primary species in each reaction
  std::vector<std::vector<bool>> _primary_participation;
  /// Stoichiometric coefficients of primary variables in each reaction
  std::vector<std::vector<Real>> _sto_u;
  /// Stoichiometric coefficients of coupled primary variables in each reaction
  std::vector<std::vector<std::vector<Real>>> _sto_v;
  /// Coupled primary species for each reaction
  std::vector<std::vector<std::vector<VariableName>>> _coupled_v;
  /// Primary species involved in the ith equilibrium reaction
  std::vector<std::vector<VariableName>> _primary_species_involved;
  /// Reaction network read from input file
  std::string _input_reactions;
  /// Vector of parsed reactions
  std::vector<std::string> _reactions;
  /// Number of reactions
  unsigned int _num_reactions;
  /// Pressure variable
  const std::vector<VariableName> _pressure_var;
  /// Gravity (default is (0, 0, 0))
  const RealVectorValue _gravity;
};
