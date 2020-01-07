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
 * ExternalProblem derived class that just exercises a portion of the new interface. It demonstrates
 * the limited interface necessary for creating a external problem wrapper.
 */
class DummyExternalProblem : public ExternalProblem
{
public:
  static InputParameters validParams();

  DummyExternalProblem(const InputParameters & params);

  virtual void externalSolve() override { _console << "Dummy External Solve!" << std::endl; }
  virtual void syncSolutions(Direction /*direction*/) override {}
  virtual bool converged() override { return true; }
};
