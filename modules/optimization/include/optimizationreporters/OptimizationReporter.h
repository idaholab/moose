//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationData.h"
#include "OptimizationReporterBase.h"

/**
 * Computes gradient and contains reporters for communicating between optimizeSolve and subapps
 */
class OptimizationReporter : public OptimizationDataTempl<OptimizationReporterBase>
{

public:
  static InputParameters validParams();
  OptimizationReporter(const InputParameters & parameters);

  virtual Real computeObjective() override;
  virtual void setMisfitToSimulatedValues() override;

protected:
  virtual void setICsandBounds() override;
};
