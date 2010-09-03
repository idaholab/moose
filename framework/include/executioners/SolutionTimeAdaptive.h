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

#ifndef SOLUTIONTIMEADAPTIVE_H
#define SOLUTIONTIMEADAPTIVE_H

#include "Moose.h"
#include "TransientExecutioner.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class SolutionTimeAdaptive;
template<>
InputParameters validParams<SolutionTimeAdaptive>();
class MooseSystem;

/**
 * SolutionTimeAdaptive executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class SolutionTimeAdaptive: public TransientExecutioner
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  SolutionTimeAdaptive(std::string name, MooseSystem & moose_system, InputParameters parameters);
  virtual ~SolutionTimeAdaptive();

protected:

  virtual void preSolve();
  virtual void postSolve();

  virtual Real computeDT();

  /**
   * Multiplier specifiying the direction the timestep is currently going.
   * Positive for up.  Negative for down.
   */
  int _direction;

  /**
   * Percentage to change the timestep by either way.
   */
  Real _percent_change;
  
  timeval _solve_start, _solve_end;

  Real _older_sol_time_vs_dt, _old_sol_time_vs_dt, _sol_time_vs_dt;
  
  bool _adapt_log;

  std::ofstream _adaptive_log;
  
};

#endif //SOLUTIONTIMEADAPTIVE_H
