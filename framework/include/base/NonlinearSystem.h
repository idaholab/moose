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

#ifndef NONLINEARSYSTEM_H
#define NONLINEARSYSTEM_H

#include "SystemBase.h"
#include "NonlinearSystemBase.h"

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblem ;-)
 */
class NonlinearSystem : public NonlinearSystemBase
{
public:
  NonlinearSystem(FEProblem & problem, const std::string & name);
  virtual ~NonlinearSystem();

  virtual TransientNonlinearImplicitSystem & sys() override { return _transient_sys; }

  virtual NumericVector<Number> & solutionOld() override { return *_transient_sys.old_local_solution; }

  virtual NumericVector<Number> & solutionOlder() override { return *_transient_sys.older_local_solution; }

private:
 TransientNonlinearImplicitSystem & _transient_sys;
};

#endif /* NONLINEARSYSTEM_H */
