//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

/**
 *  StressCorrosionCracking is a reporter that compute fracture growth size and number of cycles
 */
class CrackMeshCut3DUserObject;
class StressCorrosionCracking : public GeneralReporter
{
public:
  static InputParameters validParams();
  StressCorrosionCracking(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  /// cutter mesh object name
  const UserObjectName & _cutter_name;
  /// 3D mesh cutter object that provides active nodes
  CrackMeshCut3DUserObject * _3Dcutter;

  /// Length of crack growth for each fracture integral value
  const Real _max_growth_size;

  ///@{ Material specific scc parameters
  const Real & _k_low;
  const Real & _growth_rate_low;
  const Real & _k_high;
  const Real & _growth_rate_high;
  const Real & _growth_rate_mid_multiplier;
  const Real & _growth_rate_mid_exp_factor;
  ///@}

  /// The name of the reporter with fracture integral values
  const std::vector<Real> & _ki_vpp;
  ///@{ The variables with the x, y, z, id data where ki was computed
  const std::vector<Real> & _ki_x;
  const std::vector<Real> & _ki_y;
  const std::vector<Real> & _ki_z;
  const std::vector<Real> & _ki_id;
  ///@}

  /// timestep size to reach max_growth_size postprocessor
  Real & _corrosion_time_step;

  /// growth increment reporters
  std::vector<Real> & _growth_increment;
  ///@{ The variables with the x, y, z data for output
  std::vector<Real> & _x;
  std::vector<Real> & _y;
  std::vector<Real> & _z;
  std::vector<Real> & _id;
  ///@}
};
