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

class OptimizationData : public GeneralReporter
{
public:
  static InputParameters validParams();

  OptimizationData(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  std::vector<Point> & _measurement_points;
  std::vector<Real> & _measurement_values;
  std::vector<Real> & _simulation_values;
  std::vector<Real> & _misfit_values;

  // fixme lynn
  // maybe add method to compute misfit
  // add param names
  // add param values
};

namespace libMesh
{
void to_json(nlohmann::json & json, const Point & value);
}
