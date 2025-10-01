//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrackGrowthReporterBase.h"

/**
 *  StressCorrosionCracking is a reporter that compute fracture growth size and number of cycles
 */
class CrackMeshCut3DUserObject;
class StressCorrosionCracking : public CrackGrowthReporterBase
{
public:
  static InputParameters validParams();
  StressCorrosionCracking(const InputParameters & parameters);

protected:
  virtual void compute_growth(std::vector<int> & index) override;

  ///@{ Material specific scc parameters
  const Real & _k_low;
  const Real & _growth_rate_low;
  const Real & _k_high;
  const Real & _growth_rate_high;
  const Real & _growth_rate_mid_multiplier;
  const Real & _growth_rate_mid_exp_factor;
  ///@}

  /// timestep size to reach max_growth_size postprocessor
  Real & _corrosion_time_step;

  /// growth increment reporters
  std::vector<Real> & _growth_increment;
};
