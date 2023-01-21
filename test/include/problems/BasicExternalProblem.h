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

class BasicExternalProblem : public ExternalProblem
{
public:
  BasicExternalProblem(const InputParameters & params);
  ~BasicExternalProblem() = default;
  static InputParameters validParams();

  virtual void addExternalVariables() override;
  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

protected:
  unsigned int _heat_source_var;
};
