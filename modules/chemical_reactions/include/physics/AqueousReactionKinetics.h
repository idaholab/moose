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
 * Creates all the objects needed to solve a reaction network of chemical reactions in an aqueous
 * medium with a finite element continuous Galerkin discretization.
 */
class AqueousReactionKinetics : public ReactionKineticsPhysicsBase
{
public:
  static InputParameters validParams();

  AqueousReactionKinetics(const InputParameters & parameters);

protected:
  virtual void addAuxiliaryVariables() override;
  virtual void addFEKernels() override;

  /// Stoichiometric coefficients for each primary species (outer indexing) in each reaction
  std::vector<std::vector<Real>> _stos;
  /// Stoichiometric coefficients of primary/solver variables (outer indexing) in each reaction
  std::vector<std::vector<Real>> _sto_u;
  /// Stoichiometric coefficients of coupled primary variables (outer indexing) in each reaction
  std::vector<std::vector<std::vector<Real>>> _sto_v;
  /// Weight of each primary species (outer indexing) in each reaction
  std::vector<std::vector<Real>> _weights;
  /// log10(Equilibrium constants) for each reaction
  std::vector<Real> _log_eq_const;
  /// Equilibrium species: only one per reaction. This is a restriction of this implementation
  std::vector<VariableName> _eq_species;
  /// Vector of vectors, indexed by (i, j), of whether primary solver species 'i' is present in reaction 'j'
  std::vector<std::vector<bool>> _primary_participation;
  /// Coupled primary species for each reaction
  /// (outer indexing is primary species, then reactions then innermost is the species in the reaction)
  std::vector<std::vector<std::vector<VariableName>>> _coupled_v;
  /// Primary species involved in the ith equilibrium reaction (outer indexing)
  std::vector<std::vector<VariableName>> _solver_species_involved;

  /// Name of the pressure variable
  const std::vector<VariableName> & _pressure_var;
  /// Gravity vector
  const RealVectorValue _gravity;

};
