//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReactionKineticsPhysicsBase.h"

/**
 * Creates all the objects needed to solve a reaction network of chemical reactions in a fluid
 * medium with a continuous Galerkin finite element discretization
 */
class AqueousReactionKinetics : public ReactionKineticsPhysicsBase
{
public:
  static InputParameters validParams();

  AqueousReactionKinetics(const InputParameters & parameters);

protected:
  virtual void addAuxiliaryVariables() override;
  virtual void addFEKernels() override;

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
  /// Vector of vectors, indexed by (i, j), of whether primary solver species 'i' is present in reaction 'j'
  std::vector<std::vector<bool>> _primary_participation;
  /// Stoichiometric coefficients of primary variables in each reaction
  std::vector<std::vector<Real>> _sto_u;
  /// Stoichiometric coefficients of coupled primary variables in each reaction
  std::vector<std::vector<std::vector<Real>>> _sto_v;
  /// Coupled primary species for each reaction
  std::vector<std::vector<std::vector<VariableName>>> _coupled_v;
  /// Primary species involved in the ith equilibrium reaction
  std::vector<std::vector<VariableName>> _solver_species_involved;

  /// Name of the pressure variable
  const std::vector<VariableName> & _pressure_var;
  /// Gravity vector
  const RealVectorValue _gravity;

};
