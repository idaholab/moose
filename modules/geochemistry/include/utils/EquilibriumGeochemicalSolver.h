//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EquilibriumGeochemicalSystem.h"
#include "GeochemistryIonicStrength.h"

/**
 * This class contains methods to solve the algebraic system in EquilibriumGeochemicalSystem
 */
class EquilibriumGeochemicalSolver
{
public:
  /**
   * Construct and check for sensible arguments
   * @param mgd The ModelGeocehmicalDatabase for the model that holds the basis species names,
   * equilibrium stoichiometry, etc
   * @param egs The EquilibriumGeochemicalSystem that holds the molalities, activities, etc.
   * EquilibriumGeochemicalSolver changes quantities within egs as it solves the system.
   * @param is The object to compute ionic strengths.  EquilibriumGeochemicalSolver changes the
   * maximum value of ionic strength as it progresses towards the solution
   * @param abs_tol The Newton solution process is deemed to have converged if the L1 norm of the
   * residual < abs_tol
   * @param rel_tol The Newton solution process is deemed to have converged if the L1 norm of the
   * residual < rel_tol * residual_0
   * @param max_iter Only max_iter Newton iterations are allowed in solving the system
   * @param max_initial_residual egs guesses the initial configuration, but sometimes this results
   * in a very large initial residual.  EquilibriumGeochemicalSolver will attempt to alter the
   * initial guesses for molalities so that the initial residual is less than max_initial_residual
   * @param swap_threshold If a basis molality < swap_threshold at the end of the Newton process,
   * EquilibriumGeochemicalSolver attempts to swap it out of the basis
   * @param prevent_precipitation The minerals named in this vector will not be allowed to
   * precipitate, even if their saturation indices are positive
   * @param max_ionic_strength Maximum ionic strength ever allowed
   * @param ramp_max_ionic_strength The maximum ionic strength is ramped from 0 to
   * max_ionic_strength over the course of the first ramp_max_ionic_strength Newton iterations. This
   * can help with convergence.
   */
  EquilibriumGeochemicalSolver(const ModelGeochemicalDatabase & mgd,
                               EquilibriumGeochemicalSystem & egs,
                               GeochemistryIonicStrength & is,
                               Real abs_tol,
                               Real rel_tol,
                               unsigned max_iter,
                               Real max_initial_residual,
                               Real swap_threshold,
                               const std::vector<std::string> & prevent_precipitation,
                               Real max_ionic_strength,
                               unsigned ramp_max_ionic_strength);

  /**
   * Solve the system
   * @param ss Textual information such as (iteration, residual) and swap information is written to
   * this stringstream
   * @param tot_iter The total number of iterations used in the solve
   * @param abs_residual The residual of the algebraic system as this method exits
   */
  void solveSystem(std::stringstream & ss, unsigned & tot_iter, Real & abs_residual);

private:
  /// The database for the user-defined model
  const ModelGeochemicalDatabase & _mgd;
  /// The EquilibriumGeochmicalSystem
  EquilibriumGeochemicalSystem & _egs;
  /// The ionic-strength calculator
  GeochemistryIonicStrength & _is;
  /// Number of species in the basis
  const unsigned _num_basis;
  /// Number of species in equilibrium with the basis components
  const unsigned _num_eqm;
  /// Number of basis molalities (and potentially solvent water mass) in the algebraic system
  unsigned _num_basis_in_algebraic_system;
  /// Number of unknowns (molalities and surface potentials) in the algebraic system
  unsigned _num_in_algebraic_system;
  /// residual of the algebraic system we wish to solve
  DenseVector<Real> _residual;
  /// L1 norm of residual
  Real _abs_residual;
  /// jacobian of the algebraic system
  DenseMatrix<Real> _jacobian;
  /// the new molality after finding the solution of _jacobian * neg_change_mol = _residual
  DenseVector<Real> _new_mol;
  /// If the residual of the algebraic system falls below this value, the Newton process has converged
  const Real _abs_tol;
  /// If the residual of the algebraic system falls below this value times the initial residual, the Newton process has converged
  const Real _rel_tol;
  /// _res0_times_rel = _rel_tol * initial residual
  Real _res0_times_rel;
  /// maximum number of iterations allowed during an inner solve
  const unsigned _max_iter;
  /// maximum desired initial residual
  const Real _max_initial_residual;
  /// If a basis molality < swap_threshold, we attempt to swap it out of the basis
  const Real _swap_threshold;
  /// The minerals named in this list can have positive saturation indices and will not precipitate
  const std::vector<std::string> _prevent_precipitation;
  /// Maximum ionic strength allowed
  const Real _max_ionic_strength;
  /// Number of iterations over which to increase the maximum ionic strength to _max_ionic_strength
  const unsigned _ramp_max_ionic_strength;

  /**
   * Builds the residual of the algebraic system
   * @return the L1 norm of residual
   */
  Real computeResidual(DenseVector<Real> & residual) const;

  /**
   * Solves _jacobian * neg_change_mol = _residual for neg_change_mol, then performs an
   * underrelaxation to get new_mol
   * @param jacobian the jacobian of the system
   * @param new_mol upon exit, this will be the new molality (and surface potential values, if any)
   * values according to the underrelaxed Newton process
   */
  void solveAndUnderrelax(DenseMatrix<Real> & jacobian, DenseVector<Real> & new_mol) const;

  /**
   * Check if a basis swap is needed.  It is needed if:
   * - the Newton process did not converge and some non-minerals have small molality
   * - the free number of moles of a basis mineral is negative
   * - the saturation index of an equilibrium mineral is positive (and it is not in the
   * prevent_precipitation list)
   * @param swap_out_of_basis the index of the species in the basis that will be removed from the
   * basis
   * @param swap_into_basis the index of the equilibrium mineneral that will be added to the basis
   * @return true if a swap is needed, false otherwise
   */
  bool swapNeeded(unsigned & swap_out_of_basis,
                  unsigned & swap_into_basis,
                  std::stringstream & ss) const;

  /**
   * Progressively alter the initial-guess molalities for the algebraic system to attempt to reduce
   * the residual
   */
  bool reduceInitialResidual();
};
