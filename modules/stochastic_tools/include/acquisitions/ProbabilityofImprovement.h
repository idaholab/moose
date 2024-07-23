//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelAcquisitionFunctionBase.h"

class ProbabilityofImprovement : public ParallelAcquisitionFunctionBase
{
public:
  static InputParameters validParams();
  ProbabilityofImprovement(const InputParameters & parameters);

  /// Compute the acquisition function values
  void computeAcquisition(std::vector<Real> & acq,
                          const std::vector<Real> & gp_mean,
                          const std::vector<Real> & gp_std,
                          const std::vector<std::vector<Real>> & test_inputs,
                          const std::vector<std::vector<Real>> & train_inputs,
                          const std::vector<Real> & generic) const override;
};
