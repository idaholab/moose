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
  // fixme this should be a struct.  The order is measurement point, measurement value, simulation
  // value, misfit
  std::vector<std::tuple<Point, Real, Real, Real>> & _optimization_data;
};

namespace libMesh
{
void to_json(nlohmann::json & json, const std::vector<std::tuple<Point, Real, Real, Real>> & value);
}

/**
 * Store and load methods for measurement_data tuple,
 * fixme I don't know how to test these
 */
///@{
template <>
void dataStore(std::ostream & stream,
               std::tuple<Point, Real, Real, Real> & optimization_data_entry,
               void * context);
template <>
void dataLoad(std::istream & stream,
              std::tuple<Point, Real, Real, Real> & optimization_data_entry,
              void * context);
///@}
