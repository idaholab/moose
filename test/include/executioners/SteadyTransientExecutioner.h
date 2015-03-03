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
#ifndef STEADYTRANSIENTEXECUTIONER_H
#define STEADYTRANSIENTEXECUTIONER_H

#include "CoupledExecutioner.h"

class SteadyTransientExecutioner;

template<>
InputParameters validParams<SteadyTransientExecutioner>();

/**
 * This executioner first solves a Steady problem, then transfers the solution into the other problem and then runs the second one
 */
class SteadyTransientExecutioner : public CoupledExecutioner
{
public:
  SteadyTransientExecutioner(const std::string & name, InputParameters parameters);
  virtual ~SteadyTransientExecutioner();

  virtual void execute();

protected:

};

#endif /* STEADYTRANSIENTEXECUTIONER_H */
