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
#ifndef COUPLEDTRANSIENTEXECUTIONER_H
#define COUPLEDTRANSIENTEXECUTIONER_H

#include "CoupledExecutioner.h"

class CoupledTransientExecutioner;

template<>
InputParameters validParams<CoupledTransientExecutioner>();

/**
 * This executioner first solves a Steady problem, then transfers the solution into the other problem and then runs the second one
 */
class CoupledTransientExecutioner : public CoupledExecutioner
{
public:
  CoupledTransientExecutioner(const std::string & name, InputParameters parameters);
  virtual ~CoupledTransientExecutioner();

  virtual void execute();

protected:
  /// Current time
  Real _time;
  /// Current delta t... or timestep size.
  Real _dt;

  unsigned int _t_step;
  unsigned int _n_steps;
};

#endif /* COUPLEDTRANSIENTEXECUTIONER_H */
