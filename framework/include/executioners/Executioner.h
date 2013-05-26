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

#include "MooseObject.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"

// System includes
#include <string>

class MooseMesh;
class Problem;
class Executioner;

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
class Executioner :
  public MooseObject,
  public UserObjectInterface,
  public PostprocessorInterface
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
   * Initialize the executioner
   */
  virtual void init();

  /**
   * Pure virtual execute function MUST be overriden by children classes.
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

  virtual Problem & problem() = 0;

  void outputInitial(bool out_init);

  /**
   * Set (or reset) the output position of the application.
   */
  virtual void setOutputPosition(Point p) {}

protected:
  /// output initial condition if true
  bool _output_initial;
  /// Initial Residual Variables
  Real _initial_residual_norm;
  Real _old_initial_residual_norm;

  // Restart
  std::string _restart_file_base;
};

#endif //EXECUTIONER_H
