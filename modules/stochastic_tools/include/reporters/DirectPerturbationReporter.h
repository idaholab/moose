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
#include "VectorCalculators.h"

#include "nlohmann/json.h"

class DirectPerturbationReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  DirectPerturbationReporter(const InputParameters & parameters);

  virtual void initialize() override final;

  virtual void execute() override final {}
  virtual void finalize() override final {}

private:
  /// Direct perturbation sampler (don't need any specific functions, but should be this type)
  Sampler & _sampler;
};
