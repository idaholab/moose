//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralReporter.h"

class StochasticReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  StochasticReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  static std::string convergedReporterName() { return "multiapp_converged"; }
  void setSampleConverged(bool val, dof_id_type index) { _converged[index] = val; }

protected:
  /// Reporter value for whether or not multiapp converged
  std::vector<bool> & _converged;
};
