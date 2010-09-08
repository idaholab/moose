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

#ifndef STEADY_H
#define STEADY_H

#include "Moose.h"
#include "Executioner.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>


// Forward Declarations
class Steady;
template<>
InputParameters validParams<Steady>();

/**
 * Steady executioners usually only call "solve()" on the NonlinearSystem once.
 */
class Steady: public Executioner
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Steady(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void execute();

protected:
  /**
   * Reference to moose_system
   */
  MooseSystem & _moose_system;

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged();

  unsigned int _max_r_steps;

  /**
   * Current timestep.
   *
   * Please don't modify this directly!
   */
  int & _t_step;
};

#endif //STEADY_H
