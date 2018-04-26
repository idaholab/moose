//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LUMPEDEXPLICITEULER_H
#define LUMPEDEXPLICITEULER_H

#include "TimeIntegrator.h"

#include "libmesh/petsc_vector.h"

// Forward declarations
class LumpedExplicitEuler;
class LStableDirk4;

template <>
InputParameters validParams<LumpedExplicitEuler>();

class LumpedExplicitEuler : public TimeIntegrator
{
public:
  LumpedExplicitEuler(const InputParameters & parameters);

  virtual void preSolve() override;
  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  PetscVector<Real> & _explicit_euler_update;

  PetscVector<Real> & _mass_matrix_diag;

  TagID _Ke_time_tag;
};

#endif // LUMPEDEXPLICITEULER_H
