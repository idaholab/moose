/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef EXECUTIONER_H
#define EXECUTIONER_H

#include "ValidParams.h"
#include "MooseObject.h"
#include "FunctionInterface.h"

// System includes
#include <string>

// Forward Declarations
class Executioner;
class MooseSystem;

template<>
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
class Executioner : public MooseObject, protected FunctionInterface
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Executioner(const std::string & name, InputParameters parameters);

  virtual ~Executioner();

  /**
   * Optionally override to perform some initial setup steps.
   */
  virtual void setup();

  /**
   * Pure virtual execute function MUST be overriden by children classes.
   * This is where the Executioner actually does it's work.
   */
  virtual void execute() = 0;

  /**
   * Update newton-step related data
   */
  void updateNewtonStep();

  /**
   * Gets called at the beginning of every newton step
   */
  virtual void onNewtonUpdate() { }

  /**
   * Get MooseSystem associated with this Executioner
   */
  virtual MooseSystem & getMooseSystem() { return _moose_system; }
  
  /**
   * Optionally override to do something before execute() gets called
   */
  virtual void preExecute() {};

protected:

  /**
   * Optionally override to do something after execute() gets called
   */
  virtual void postExecute() {};

  /**
   * Optionally override to do something before solve() gets called
   */
  virtual void preSolve() {};

  /**
   * Optionally override to do something after solve() gets called
   * but before adaptivity.  Might be useful for postprocessing.
   */
  virtual void postSolve();

  /**
   * Should be overriden if possible.  Should return true if the last solve
   * that was performed converged.  Should return false otherwise.
   */
  virtual bool lastSolveConverged(){ return true; }

  /**
   * Adapt the mesh based on parameters set in the input file.
   *
   * This is here because every Executioner object should have
   * adaptivity capability.
   */
  virtual void adaptMesh();

  /**
   * Should be called before solve() to set the relative equation scaling.
   */
  void setScaling();

  MooseSystem & _moose_system;

  /**
   * Initial Residual Variables
   */
  Real _initial_residual_norm;
  Real _old_initial_residual_norm;

};

#endif //EXECUTIONER_H
