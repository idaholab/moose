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
 * FEProblemBase derived class that will fail a prescribed timestep for testing
 * timestepping algorithms
 */
class FailingProblem : public FEProblem
{
public:
  static InputParameters validParams();

  FailingProblem(const InputParameters & params);
  virtual bool converged();

protected:
  std::vector<unsigned int> _fail_steps;
};
