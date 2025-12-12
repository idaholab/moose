//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExplicitMixedOrder.h"
#include "PostprocessorInterface.h"

/**
 * ExplicitMixedOrderStasis time integrator, with the additional ability to perform time steps in
 * which velocity and acceleration are set to zero, and the primary variables are not changed
 */
class ExplicitMixedOrderStasis : public ExplicitMixedOrder, PostprocessorInterface
{
public:
  static InputParameters validParams();

  ExplicitMixedOrderStasis(const InputParameters & parameters);

  virtual void init() override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

  virtual bool performExplicitSolve(SparseMatrix<Number> & mass_matrix) override;

protected:
  const PostprocessorValue & _in_stasis;

  // for recording _dt_old_to_use, need to keep track of whether previous time step was in stasis
  bool _prev_timestep_in_stasis;

  /*
  The time step immediately following stasis cannot use _dt_old in the central differencing,
  because that _dt_old might have been huge.  _dt_old_to_use is the _dt_old encountered in
  the most recent period of non-stasis
  */
  Real _dt_old_to_use;

  void computeICs() override;
  virtual Real centralDifferenceDt() override;
};
