//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ACTUALLYEXPLICITEULER_H
#define ACTUALLYEXPLICITEULER_H

#include "TimeIntegrator.h"

#include "libmesh/linear_solver.h"

// Forward declarations
class ActuallyExplicitEuler;
class LStableDirk4;
class LumpedPreconditioner;

template <>
InputParameters validParams<ActuallyExplicitEuler>();

class ActuallyExplicitEuler : public TimeIntegrator
{
public:
  ActuallyExplicitEuler(const InputParameters & parameters);

  virtual void init() override;
  virtual void preSolve() override;
  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  enum SolveType
  {
    CONSISTENT,
    LUMPED,
    LUMP_PRECONDITIONED
  };

  MooseEnum _solve_type;

  NumericVector<Real> & _explicit_euler_update;

  NumericVector<Real> & _mass_matrix_diag;

  NumericVector<Real> * _ones;

  TagID _Ke_time_tag;

  std::unique_ptr<LinearSolver<Number>> _linear_solver;

  std::unique_ptr<LumpedPreconditioner> _preconditioner;
};

#endif // ACTUALLYEXPLICITEULER_H
