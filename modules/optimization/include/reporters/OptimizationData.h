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
  std::vector<Real> & _measurement_xcoord;
  std::vector<Real> & _measurement_ycoord;
  std::vector<Real> & _measurement_zcoord;
  std::vector<Real> & _measurement_time;
  std::vector<Real> & _measurement_values;
  std::vector<Real> & _simulation_values;
  std::vector<Real> & _misfit_values;

  // fixme lynn
  // maybe add method to compute misfit
  // add param names
  // add param values
private:
  void readMeasurementsFromFile();
  void readMeasurementsFromInput();
  void setSimuilationValuesForTesting(std::vector<Real> & data);

  const MooseVariableFieldBase * const _var;
};
