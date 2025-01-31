//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TransientBase.h"
#include "PIMPLESolve.h"

/**
 * Executioner set up to solve a transient thermal-hydraulics problem using the PIMPLE algorithm.
 * It utilizes segregated linear systems which are solved using a fixed-point iteration within time
 * steps.
 */
class PIMPLE : public TransientBase
{
public:
  static InputParameters validParams();

  PIMPLE(const InputParameters & parameters);

  virtual void init() override;

  virtual Real relativeSolutionDifferenceNorm() override;

protected:
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const override;

  /// The solve object performing the PIMPLE iteration
  PIMPLESolve _pimple_solve;
};
