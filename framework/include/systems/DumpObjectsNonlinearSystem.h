//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DUMPOBJECTSNONLINEARSYSTEM_H
#define DUMPOBJECTSNONLINEARSYSTEM_H

#include "NonlinearSystemBase.h"
#include "libmesh/nonlinear_solver.h"

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Nonlinear system for dumping objects
 */
class DumpObjectsNonlinearSystem : public NonlinearSystemBase
{
public:
  DumpObjectsNonlinearSystem(FEProblemBase & problem, const std::string & name);

  virtual NonlinearSolver<Number> * nonlinearSolver() override { return NULL; }
  virtual void solve() override { }
  virtual void stopSolve() override { }
  virtual bool converged() override { return true; }
  virtual NumericVector<Number> & RHS() override { return *_dummy; }
  virtual NumericVector<Number> & solutionOld() override { return *_dummy; }
  virtual NumericVector<Number> & solutionOlder() override { return *_dummy; }
  virtual unsigned int getCurrentNonlinearIterationNumber() override { return 0; }
  virtual void setupFiniteDifferencedPreconditioner() override { }

protected:
  NumericVector<Number> * _dummy;
};

#endif /* DUMPOBJECTSNONLINEARSYSTEM_H */
