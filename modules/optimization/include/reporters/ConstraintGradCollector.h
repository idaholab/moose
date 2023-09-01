//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

class ConstraintGradCollector : public GeneralReporter
{
public:
  static InputParameters validParams();
  ConstraintGradCollector(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

private:
  /// reporter that holds all the parameters
  const std::vector<Real> _parameters;
  /// holds all the sub-gradient vectors
  std::vector<std::vector<const std::vector<Real> *>> _gradient_vecs;
  /// holds all the total gradient vectors
  std::vector<std::vector<Real> *> _total_vecs;
};
