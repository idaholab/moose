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

  virtual libMesh::NonlinearSolver<Number> * nonlinearSolver() override { return NULL; }
  virtual void solve() override {}
  virtual void stopSolve(const ExecFlagType &, const std::set<TagID> &) override {}
  virtual bool converged() override { return true; }
  virtual NumericVector<Number> & RHS() override { return *_dummy; }
  virtual SNES getSNES() override { return nullptr; }

  virtual unsigned int getCurrentNonlinearIterationNumber() override { return 0; }
  virtual void setupFiniteDifferencedPreconditioner() override {}
  virtual void attachPreconditioner(libMesh::Preconditioner<Number> * /* preconditioner */) override
  {
  }
  void residualAndJacobianTogether() override {}

protected:
  void computeScalingJacobian() override {}
  void computeScalingResidual() override {}

  NumericVector<Number> * _dummy;
};
