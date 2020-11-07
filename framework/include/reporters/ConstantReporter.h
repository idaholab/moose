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

class ConstantReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  ConstantReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override {}

protected:
  std::vector<int *> _int;
  std::vector<Real *> _real;
  std::vector<std::vector<Real> *> _vector;
  std::vector<std::string *> _string;
};
