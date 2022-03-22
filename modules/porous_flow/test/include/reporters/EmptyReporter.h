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

class EmptyReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  EmptyReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  const std::vector<Real> & _coord_x;
  const std::vector<Real> & _coord_y;
  const std::vector<Real> & _coord_z;
  const std::vector<Real> & _weight;
};
