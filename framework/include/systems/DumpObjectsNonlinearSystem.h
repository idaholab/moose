//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
  virtual void solve() override {}
  virtual void stopSolve() override {}
  virtual bool converged() override { return true; }
  virtual NumericVector<Number> & RHS() override { return *_dummy; }

  virtual unsigned int getCurrentNonlinearIterationNumber() override { return 0; }
  virtual void setupFiniteDifferencedPreconditioner() override {}
  virtual void attachPreconditioner(Preconditioner<Number> * /* preconditioner */) override {}

protected:
  NumericVector<Number> & solutionOldInternal() const override { return *_dummy; }
  NumericVector<Number> & solutionOlderInternal() const override { return *_dummy; }

  void computeScalingJacobian() override {}
  void computeScalingResidual() override {}

  NumericVector<Number> * _dummy;
};
