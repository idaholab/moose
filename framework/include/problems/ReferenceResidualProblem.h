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

/**
 * FEProblemBase derived class to enable convergence checking relative to a user-specified
 * postprocessor
 */
class ReferenceResidualProblem : public FEProblem
{
public:
  static InputParameters validParams();

  ReferenceResidualProblem(const InputParameters & params);

  virtual void addDefaultConvergence() override;

protected:
};
