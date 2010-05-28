#ifndef EXECUTIONER_H
#define EXECUTIONER_H

#include "ValidParams.h"

// LibMesh includes
#include "parameters.h"
#include "point.h"
#include "vector_value.h"
#include "nonlinear_implicit_system.h"
#include "transient_system.h"

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
class Executioner
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Executioner(std::string name, MooseSystem & moose_system, InputParameters parameters);

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

protected:

  /**
   * The reference to the encompassing Moose System
   */
  MooseSystem & _moose_system;

  /**
   * Optionally override to do something before solve() gets called
   */
  virtual void preSolve() {};

  /**
   * Optionally override to do something after solve() gets called
   * but before adaptivity.  Might be useful for postprocessing.
   */
  virtual void postSolve() {};

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

  std::string _name;
  InputParameters _parameters;

  /**
   * The Nonlinear System
   */
//  TransientNonlinearImplicitSystem & _system;

  /**
   * The Auxiliary System
   */
//  TransientExplicitSystem & _aux_system;

  /**
   * Initial Residual Variables
   */
  Real _initial_residual_norm;
  Real _old_initial_residual_norm;
};

#endif //EXECUTIONER_H
