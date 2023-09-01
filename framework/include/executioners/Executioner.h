//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "Restartable.h"
#include "PerfGraphInterface.h"
#include "FEProblemSolve.h"
#include "FixedPointSolve.h"
#include "PicardSolve.h"
#include "Reporter.h"
#include "ReporterInterface.h"

// System includes
#include <string>

class Problem;
/**
 * Executioners are objects that do the actual work of solving your problem.
 */
class Executioner : public MooseObject,
                    private Reporter,          // see addAttributeReporter
                    private ReporterInterface, // see addAttributeReporter
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

  /// executor-style constructor that skips the fixed point solve object
  /// allocation.
  Executioner(const InputParameters & parameters, bool);

  virtual ~Executioner() {}

  static InputParameters validParams();

  /**
   * Perform initializations during executing actions right before init_problem task
   */
  virtual void preProblemInit() {}

  /**
   * Initialize the executioner
   */
  virtual void init() {}

  /**
   * Pure virtual execute function MUST be overridden by children classes.
   * This is where the Executioner actually does it's work.
   */
  virtual void execute() = 0;

  /**
   * Override this for actions that should take place before execution
   */
  virtual void preExecute() {}

  /**
   * Override this for actions that should take place after execution
   */
  virtual void postExecute() {}

  /**
   * Override this for actions that should take place before execution, called by PicardSolve
   */
  virtual void preSolve() {}

  /**
   * Override this for actions that should take place after execution, called by PicardSolve
   */
  virtual void postSolve() {}

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
  virtual std::string getTimeStepperName() const { return std::string(); }

  /** The name of the TimeIntegrator
   * This is an empty string for non-Transient executioners
   * @return A string of giving the TimeIntegrator name
   */
  virtual std::string getTimeIntegratorName() const { return std::string(); }

  /**
   * Can be used by subsclasses to call parentOutputPositionChanged()
   * on the underlying FEProblemBase.
   */
  virtual void parentOutputPositionChanged() {}

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged() const = 0;

  /// Return underlying PicardSolve object.
  PicardSolve & picardSolve()
  {
    mooseDeprecated("picardSolve() is deprecated. Use FixedPointSolve() instead.");
    if (_iteration_method == "picard")
      return *(dynamic_cast<PicardSolve *>(_fixed_point_solve.get()));
    else
      mooseError("Cannot return a PicardSolve if the iteration method is not Picard.");
  }

  FixedPointSolve & fixedPointSolve() { return *_fixed_point_solve; }

  /// Augmented Picard convergence check to be called by PicardSolve and can be overridden by derived executioners
  virtual bool augmentedPicardConvergenceCheck() const
  {
    mooseDeprecated(
        "augmentedPicardConvergenceCheck() is deprecated. Use augmentedCouplingConvergenceCheck.");
    return false;
  }

  /**
   * Get the verbose output flag
   * @return The verbose output flag
   */
  const bool & verbose() const { return _verbose; }

  /**
   * Return supported iteration methods that can work with MultiApps on timestep_begin and
   * timestep_end
   */
  static MooseEnum iterationMethods() { return MooseEnum("picard secant steffensen", "picard"); }

  /**
   * Get a general iteration number for the purpose of outputting, useful in the presence of a
   * nested solve. This output iteration number may be set by the parent app for a sub-app. This
   * behavior is decided by the Executioner/Executor/SolveObject in charge of the solve.
   */
  virtual unsigned int getIterationNumberOutput() const { return _output_iteration_number; }

  /**
   * Set a general iteration number for the purpose of outputting, useful in the presence of a
   * nested solve This output iteration number may be set by the parent app for a sub-app, e.g.
   * OptimizeSolve.
   *
   * @param iteration_number The iteration number.
   */
  virtual void setIterationNumberOutput(unsigned int iteration_number)
  {
    _output_iteration_number = iteration_number;
  }

  /**
   * Set whether we are solving with outer iterations (e.g. optimization).
   *
   * @param is_outer_iteration_solve Whether the solution process contains outer iterations.
   */
  virtual void setIterationOutputFlag(const bool is_outer_iteration_solve)
  {
    _is_outer_iteration_solve = is_outer_iteration_solve;
  }

  /**
   * Get whether we are solving with outer iterations (e.g. optimization).
   *
   * @return Whether the solution process contains outer iterations.
   */
  virtual bool getIterationOutputFlag() const { return _is_outer_iteration_solve; }

protected:
  /**
   * Adds a postprocessor that the executioner can directly assign values to
   * @param name The name of the postprocessor to create
   * @param initial_value The initial value of the postprocessor value
   * @return Reference to the postprocessor data that to be used by this executioner
   */
  virtual PostprocessorValue & addAttributeReporter(const std::string & name,
                                                    Real initial_value = 0);

  FEProblemBase & _fe_problem;

  MooseEnum _iteration_method;
  std::unique_ptr<FixedPointSolve> _fixed_point_solve;

  // Restart
  std::string _restart_file_base;

  /// True if printing out additional information
  const bool & _verbose;

  /// Iteration number obtained from the main application
  unsigned int _output_iteration_number;

private:
  /// Whether we are solving an outer-iteration problem (e.g. optimization).
  bool _is_outer_iteration_solve;
};
