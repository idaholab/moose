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

class TestReporterPartitioning : public GeneralReporter
{
public:
  static InputParameters validParams();
  TestReporterPartitioning(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

private:
  const Sampler & _sampler;
  std::unordered_map<ReporterName, const std::vector<Real> *> _reporters;
};
