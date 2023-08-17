//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationDataHelper.h"
#include "OptimizationReporterBase.h"

/**
 * Computes gradient and contains reporters for communicating between optimizeSolve and subapps
 */
class OptimizationReporter : public OptimizationReporterBase
{
private:
  /// Data helper for generating objective value.
  OptimizationDataHelper _opt_data;

public:
  static InputParameters validParams();
  OptimizationReporter(const InputParameters & parameters);

  void execute() override;

  virtual Real computeObjective() override;
  virtual void setMisfitToSimulatedValues() override;

  /// measurement values
  std::vector<Real> & _measurement_values;
  /// simulated values at measurement xyzt
  std::vector<Real> & _simulation_values;
  /// difference between simulation and measurement values at measurement xyzt
  std::vector<Real> & _misfit_values;

private:
  virtual void setICsandBounds() override;
  virtual void setSimulationValuesForTesting(std::vector<Real> & data) override;
};
