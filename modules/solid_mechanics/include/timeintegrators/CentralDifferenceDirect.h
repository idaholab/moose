//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExplicitTimeIntegrator.h"
#include "libmesh/sparse_matrix.h"

/**
 * Implements a truly explicit (no nonlinear solve) first-order, forward Euler
 * time integration scheme.
 */
class CentralDifferenceDirect : public ExplicitTimeIntegrator
{
public:
  static InputParameters validParams();

  CentralDifferenceDirect(const InputParameters & parameters);

  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;

  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

  virtual bool performExplicitSolve(SparseMatrix<Number> & mass_matrix) override;

  // // Applying pre-set nodal BCs
  // void setPresetBCs();

  void computeADTimeDerivatives(ADReal &, const dof_id_type &, ADReal &) const override
  {
    mooseError("NOT SUPPORTED");
  }

protected:
  /**
   * Helper function that actually does the math for computing the time derivative
   */

  const bool & _constant_mass;

  const TagName & _mass_matrix;
};
