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

#ifndef FAILINGPROBLEM_H
#define FAILINGPROBLEM_H

#include "FEProblem.h"

class FailingProblem;

template<>
InputParameters validParams<FailingProblem>();

/**
 * FEProblem derived class that will fail a prescribed timestep for testing
 * timestepping algorithms
 */
class FailingProblem : public FEProblem
{
public:
  FailingProblem(const std::string & name, InputParameters params);
  virtual bool converged();
  
protected:
  bool _failed;
  unsigned int _fail_step;
};

#endif /* FAILINGPROBLEM_H */
