//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemicalModelDefinition.h"
#include "EquilibriumGeochemicalSystem.h"
#include "EquilibriumGeochemicalSolver.h"
#include "Output.h"
#include "UserObjectInterface.h"

/**
 * Solves a reaction system for molalities and outputs results
 */
class EquilibriumReactionSolverOutput : public Output, public UserObjectInterface
{
public:
  static InputParameters validParams();

  EquilibriumReactionSolverOutput(const InputParameters & parameters);

protected:
  virtual void output(const ExecFlagType & type) override;

  /// my copy of the underlying ModelGeochemicalDatabase
  ModelGeochemicalDatabase _mgd;
  /// number of basis species
  const unsigned _num_basis;
  /// number of equilibrium species
  const unsigned _num_eqm;
  /// The species swapper
  GeochemistrySpeciesSwapper _swapper;
  /// Initial value of maximum ionic strength
  const Real _initial_max_ionic_str;
  /// The ionic strength calculator
  GeochemistryIonicStrength _is;
  /// The activity calculator
  GeochemistryActivityCoefficientsDebyeHuckel _gac;
  /// The equilibrium geochemical system that holds all the molalities, activities, etc
  EquilibriumGeochemicalSystem _egs;
  /// The solver
  EquilibriumGeochemicalSolver _solver;
  /// The left-hand side specified in the original model definition for redox half reactions
  const std::string _original_redox_lhs;
  /// precision of output
  const unsigned _precision;
  /// Tolerance on stoichiometric coefficients before they are deemed to be zero
  const Real _stoi_tol;
  /// Whether to print iteration residuals, swap information, etc
  const bool _verbose;
  /// Species with molalities less than mol_cutoff will not be outputted
  const Real _mol_cutoff;
  /// Temperature specified by user
  const Real _temperature;
  /// Species to swap out of basis prior to outputting the Nernst potentials
  const std::vector<std::string> _nernst_swap_out_of_basis;
  /// Species to swap into basis prior to outputting the Nernst potentials
  const std::vector<std::string> _nernst_swap_into_basis;

private:
  /**
   * Perform the swaps specified by the user, prior to outputting Nernst redox information.  In
   * addition, use swaps to attempt to make the left-hand side of the redox half reaction equal to
   * _original_redox_lhs
   */
  void performNernstSwaps();
};
