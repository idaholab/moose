//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

// Forward declaration
class EulerAngleUpdater;
class EulerAngleProvider;
class GrainTrackerInterface;
class GrainForceAndTorqueInterface;
class RotationTensor;

/**
 * This is a unit test to check the correctness of the updated euler angles
 * An unit vector is rotated as per old euler angles first and then due to the applied torque
 * The final rotated vector is cross checked with the rotated vector as per updated euler angles
 */
class EulerAngleUpdaterCheck : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  EulerAngleUpdaterCheck(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  VectorPostprocessorValue & _diff;

protected:
  const GrainTrackerInterface & _grain_tracker;
  const EulerAngleUpdater & _euler;
  const GrainForceAndTorqueInterface & _grain_torque;
  const VectorPostprocessorValue & _grain_volumes;

  const Real _mr;

  std::vector<RealVectorValue> _angles;
  std::vector<RealVectorValue> _angles_old;
};
