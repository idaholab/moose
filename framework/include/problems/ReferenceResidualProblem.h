//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"
#include "ReferenceResidualInterface.h"

/**
 * Problem that checks for convergence relative to a user-supplied reference quantity
 * rather than the initial residual.
 */
class ReferenceResidualProblem : public FEProblem, public ReferenceResidualInterface
{
public:
  static InputParameters validParams();

  ReferenceResidualProblem(const InputParameters & params);

  virtual void addDefaultNonlinearConvergence(const InputParameters & params) override;
  virtual bool onlyAllowDefaultNonlinearConvergence() const override { return true; }
};
