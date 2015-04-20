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

#ifndef COUPLEDPROBLEM_H
#define COUPLEDPROBLEM_H

#include "Problem.h"

class CoupledProblem;

template<>
InputParameters validParams<CoupledProblem>();

class CoupledProblem : public Problem
{
public:
  CoupledProblem(const std::string & name, InputParameters params);
  virtual ~CoupledProblem();

  virtual void init();

protected:
};

#endif /* COUPLEDPROBLEM_H */
