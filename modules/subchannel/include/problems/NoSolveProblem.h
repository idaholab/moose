//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExternalProblem.h"

/**
 * A Problem that does not solve, for visualization purposes
 */

class NoSolveProblem : public ExternalProblem
{
public:
  NoSolveProblem(const InputParameters & params);
  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool solverSystemConverged(const unsigned int) override;
  static InputParameters validParams();
};
