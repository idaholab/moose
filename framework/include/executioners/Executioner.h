//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXECUTIONER_H
#define EXECUTIONER_H

#include "MooseObject.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "Restartable.h"
#include "PerfGraphInterface.h"

// System includes
#include <string>

class Problem;
class Executioner;

template <>
InputParameters validParams<Executioner>();

/**
 * Executioners are objects that do the actual work of solving your problem.
 *
 * In general there are two "sets" of Executioners: Steady and Transient.
 *
 * The Difference is that a Steady Executioner usually only calls "solve()"
 * for the NonlinearSystem once... where Transient Executioners call solve()
 * multiple times... i.e. once per timestep.
 */
class Executioner : public MooseObject,
                    public UserObjectInterface,
                    public PostprocessorInterface,
                    public Restartable,
                    public PerfGraphInterface
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  Executioner(const InputParameters & parameters);

  virtual ~Executioner();

  /**
   * Initialize the executioner
   */
  virtual void init();

  /**
   * Pure virtual execute function MUST be overridden by children classes.
   * This is where the Executioner actually does it's work.
   */
  virtual void execute() = 0;

  /**
   * Override this for actions that should take place before execution
   */
  virtual void preExecute();

  /**
   * Override this for actions that should take place after execution
   */
  virtual void postExecute();

  /**
   * Override this for actions that should take place before execution
   */
  virtual void preSolve();

  /**
   * Override this for actions that should take place after execution
   */
  virtual void postSolve();

  /**
   * Deprecated:
   * Return a reference to this Executioner's Problem instance
   */
  virtual Problem & problem();

  /**
   * Return a reference to this Executioner's FEProblemBase instance
   */
  FEProblemBase & feProblem();

  /** The name of the TimeStepper
   * This is an empty string for non-Transient executioners
   * @return A string of giving the TimeStepper name
   */
  virtual std::string getTimeStepperName();

  /**
   * Can be used by subsclasses to call parentOutputPositionChanged()
   * on the underlying FEProblemBase.
   */
  virtual void parentOutputPositionChanged() {}

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged();

  class PicardSolve : public MooseObject
  {
    PicardSolve(const InputParameters & parameters);

    bool solve() { return true; }

  protected:
    /// Reference to FEProblem
    FEProblemBase & _problem;

    /// Maximum Picard iterations
    unsigned int _picard_max_its;
    /// Relative tolerance on residual norm
    Real _picard_rel_tol;
    /// Absolute tolerance on residual norm
    Real _picard_abs_tol;
    /// Whether or not we force evaluation of residual norms even without multiapps
    bool _picard_force_norms;
    /// Relaxation factor for Picard Iteration
    Real _relax_factor;
    /// The transferred variables that are going to be relaxed
    std::vector<std::string> _relaxed_vars;

    /// Maximum number of xfem updates per step
    unsigned int _max_xfem_update;
    /// Controls whether xfem should update the mesh at the beginning of the time step
    bool _update_xfem_at_timestep_begin;

  private:
    ///@{ Variables used by the Picard iteration
    /// Picard iteration counter
    unsigned int _picard_it;
    /// Initial residual norm
    Real _picard_initial_norm;
    /// Full history of residual norm after evaluation of timestep_begin
    std::vector<Real> _picard_timestep_begin_norm;
    /// Full history of residual norm after evaluation of timestep_end
    std::vector<Real> _picard_timestep_end_norm;
  };

protected:
  /**
   * Adds a postprocessor to report a Real class attribute
   * @param name The name of the postprocessor to create
   * @param attribute The Real class attribute to report
   * @param execute_on When to execute the postprocessor that is created
   */
  virtual void addAttributeReporter(const std::string & name,
                                    Real & attribute,
                                    const std::string execute_on = "");

  FEProblemBase & _fe_problem;

  /// Initial Residual Variables
  Real _initial_residual_norm;
  Real _old_initial_residual_norm;

  // Restart
  std::string _restart_file_base;

  // Splitting
  std::vector<std::string> _splitting;
};

#endif // EXECUTIONER_H
