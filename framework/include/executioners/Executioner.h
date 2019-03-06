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
#include "FEProblemSolve.h"
#include "PicardSolve.h"

// System includes
#include <string>

class Problem;
class Executioner;

template <>
InputParameters validParams<Executioner>();

/**
 * Executioners are objects that do the actual work of solving your problem.
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

  virtual ~Executioner() {}

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
  virtual std::string getTimeStepperName() { return std::string(); }

  /**
   * Can be used by subsclasses to call parentOutputPositionChanged()
   * on the underlying FEProblemBase.
   */
  virtual void parentOutputPositionChanged() {}

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged() const = 0;

  /// Return the underlining FEProblemSolve object.
  FEProblemSolve & feProblemSolve() { return _feproblem_solve; }

  /// Return underlining PicardSolve object.
  PicardSolve & picardSolve() { return _picard_solve; }

  /// Augmented Picard convergence check that to be called by PicardSolve and can be overridden by derived executioners
  virtual bool augmentedPicardConvergenceCheck() const { return false; }

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

  FEProblemSolve _feproblem_solve;
  PicardSolve _picard_solve;

  // Restart
  std::string _restart_file_base;
};

#endif // EXECUTIONER_H
