//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TransientBase.h"
#include "ReducedPressurePIMPLESolve.h"

/**
 * Executioner solving a transient diffuse-interface two-phase problem in the reduced pressure
 * with the PIMPLE algorithm, using segregated linear systems solved by fixed-point iteration
 * within each time step.
 */
class ReducedPressurePIMPLE : public TransientBase
{
public:
  static InputParameters validParams();

  ReducedPressurePIMPLE(const InputParameters & parameters);

  virtual void init() override;
  virtual void takeStep(Real input_dt = -1.0) override;

  virtual Real relativeSolutionDifferenceNorm(bool check_aux) const override;

protected:
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const override;

  /// The solve object performing the reduced-pressure PIMPLE iteration
  ReducedPressurePIMPLESolve _solve;
};
