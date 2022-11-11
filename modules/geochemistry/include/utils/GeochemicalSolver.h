//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemicalSystem.h"
#include "GeochemistryIonicStrength.h"

/**
 * This class contains methods to solve the algebraic system in GeochemicalSystem
 */
class GeochemicalSolver
{
public:
  /**
   * Construct and check for sensible arguments
   * @param num_basis Number of basis species in the system
   * @param mum_kin Number of kinetic species in the system
   * @param is The object to compute ionic strengths.  GeochemicalSolver changes the
   * maximum value of ionic strength as it progresses towards the solution
   * @param abs_tol The Newton solution process is deemed to have converged if the L1 norm of the
   * residual < abs_tol
   * @param rel_tol The Newton solution process is deemed to have converged if the L1 norm of the
   * residual < rel_tol * residual_0
   * @param max_iter Only max_iter Newton iterations are allowed in solving the system
   * @param max_initial_residual egs guesses the initial configuration, but sometimes this results
   * in a very large initial residual.  GeochemicalSolver will attempt to alter the
   * initial guesses for molalities so that the initial residual is less than max_initial_residual.
   * This parameter may be varied during runtime which might be useful in time-dependent simulations
   * if the first residual is poor but the remainder are deemed likely to be OK
   * @param swap_threshold If a basis molality < swap_threshold at the end of the Newton process,
   * GeochemicalSolver attempts to swap it out of the basis
   * @param max_swaps_allowed Maximum number of swaps allowed before the solve process aborts
   * @param prevent_precipitation The minerals named in this vector will not be allowed to
   * precipitate, even if their saturation indices are positive
   * @param max_ionic_strength Maximum ionic strength ever allowed
   * @param ramp_max_ionic_strength The maximum ionic strength is ramped from 0 to
   * max_ionic_strength over the course of the first ramp_max_ionic_strength Newton iterations. This
   * can help with convergence.
   * @param evaluate_kin_always If true then evaluate the kinetic rates before each residual
   * calculation, otherwise evaluate them only at the start of the solve process (which will be
   * using the molalities, etc from the previous time-step)
   */
  GeochemicalSolver(unsigned num_basis,
                    unsigned num_kin,
                    GeochemistryIonicStrength & is,
                    Real abs_tol,
                    Real rel_tol,
                    unsigned max_iter,
                    Real max_initial_residual,
                    Real swap_threshold,
                    unsigned max_swaps_allowed,
                    const std::vector<std::string> & prevent_precipitation,
                    Real max_ionic_strength,
                    unsigned ramp_max_ionic_strength,
                    bool evaluate_kin_always);

  /**
   * Solve the system
   * @param egs The geochemical system that needs to be solved.  This gets modified as the solve
   * progresses
   * @param ss Textual information such as (iteration, residual) and swap information is written to
   * this stringstream
   * @param tot_iter The total number of iterations used in the solve
   * @param abs_residual The residual of the algebraic system as this method exits
   * @param mole_additions the increment of mole number of each basis species and kinetic species
   * since the last timestep.  This must have size _num_basis + _num_kin.  For the basis species,
   * this is the amount of each species being injected into the system over the timestep.  For the
   * kinetic species, this is -dt*reaction_rate.  Please note: do not decompose the kinetic-species
   * additions into basis components and add them to the first slots of mole_additions!  This method
   * does that decomposition automatically.  The first _num_basis slots of mole_additions contain
   * kinetic-independent additions, while the last _num_kin slots contain kinetic-rate
   * contributions.  This may change during the solution process as kinetic rates depend on
   * molalities.
   * @param dmole_additions dmole_additions(a, b) = d(mole_additions(a))/d(basis_molality(b)) for b
   * < _num_basis and d(mole_additions(a))/d(kinetic_moles(b - _num_basis)) otherwise.
   */
  void solveSystem(GeochemicalSystem & egs,
                   std::stringstream & ss,
                   unsigned & tot_iter,
                   Real & abs_residual,
                   Real dt,
                   DenseVector<Real> & mole_additions,
                   DenseMatrix<Real> & dmole_additions);

  /// Set value for max_initial_residual
  void setMaxInitialResidual(Real max_initial_residual);

  /// Get value for max_initial_residual
  Real getMaxInitialResidual() const;

  /// Sets the value of _ramp_max_ionic_strength
  void setRampMaxIonicStrength(unsigned ramp_max_ionic_strength);

  /// Gets the value of _ramp_max_ionic_strength
  unsigned getRampMaxIonicStrength() const;

private:
  /// The ionic-strength calculator
  GeochemistryIonicStrength & _is;
  /// Number of species in the basis
  const unsigned _num_basis;
  /// Number of kinetic species
  const unsigned _num_kin;
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
  Real _max_initial_residual;
  /// If a basis molality < swap_threshold, we attempt to swap it out of the basis
  const Real _swap_threshold;
  /// Maximum number of swaps allowed before the solve aborts
  const unsigned _max_swaps_allowed;
  /// The minerals named in this list can have positive saturation indices and will not precipitate
  const std::vector<std::string> _prevent_precipitation;
  /// Maximum ionic strength allowed
  const Real _max_ionic_strength;
  /// Number of iterations over which to increase the maximum ionic strength to _max_ionic_strength
  unsigned _ramp_max_ionic_strength;
  /**
   * When to compute the kinetic rates: if true then evaluate before every residual calculation,
   * otherwise evaluate only at the start of the solve process (using, eg, the old molality values)
   */
  bool _evaluate_kin_always;
  /// the mole_additions for the basis and kinetic species as specified in the argument of solveSystem
  DenseVector<Real> _input_mole_additions;
  /// d(mole_additions)/d(species) as specified in the argument of solveSystem
  DenseMatrix<Real> _input_dmole_additions;

  /**
   * Builds the residual of the algebraic system
   * @param egs The geochemical system to use to compute the residual
   * @param residual The residual components will be placed here
   * @param mole_additions the increment of mole number of each basis species and kinetic species
   * since the last timestep.
   * @return the L1 norm of residual
   */
  Real computeResidual(const GeochemicalSystem & egs,
                       DenseVector<Real> & residual,
                       const DenseVector<Real> & mole_additions) const;

  /**
   * Solves _jacobian * neg_change_mol = _residual for neg_change_mol, then performs an
   * underrelaxation to get new_mol
   * @param egs The geochemical system that we're trying to solve
   * @param jacobian the jacobian of the system
   * @param new_mol upon exit, this will be the new molality (and surface potential values, if any)
   * values according to the underrelaxed Newton process
   */
  void solveAndUnderrelax(const GeochemicalSystem & egs,
                          DenseMatrix<Real> & jacobian,
                          DenseVector<Real> & new_mol) const;

  /**
   * Check if a basis swap is needed.  It is needed if:
   * - the Newton process did not converge and some non-minerals have small molality
   * - the free number of moles of a basis mineral is negative
   * - the saturation index of an equilibrium mineral is positive (and it is not in the
   * prevent_precipitation list)
   * @param egs Geochemical system that we're trying to solve
   * @param swap_out_of_basis the index of the species in the basis that will be removed from the
   * basis
   * @param swap_into_basis the index of the equilibrium mineneral that will be added to the basis
   * @return true if a swap is needed, false otherwise
   */
  bool swapNeeded(const GeochemicalSystem & egs,
                  unsigned & swap_out_of_basis,
                  unsigned & swap_into_basis,
                  std::stringstream & ss) const;

  /**
   * Progressively alter the initial-guess molalities for the algebraic system to attempt to reduce
   * the residual
   * @param egs The GeochemicalSystem we're trying to solve
   * @param dt time-step size (used for determining kinetic rates)
   * @param mole_additions the increment of mole number of each basis species and kinetic species
   * since the last timestep.  This may change during this function as kinetic rates depend on
   * molalities
   * @param dmole_additions d(mole_additions)/(molality and kinetic moles)
   */
  bool reduceInitialResidual(GeochemicalSystem & egs,
                             Real dt,
                             DenseVector<Real> & mole_additions,
                             DenseMatrix<Real> & dmole_additions);
};
