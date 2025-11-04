//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EulerAngleProvider.h"

// Forward declaration
class RotationTensor;
class GrainTrackerInterface;
class GrainForceAndTorqueInterface;

/**
 * Update Euler angles of each grains after rigid body rotation
 * This class estimates the rotation of principal axes of the grains due to applied torques
 * and calculates the final grain orientation.
 * Step1: Calculate RotationTensor based on euler angles from previous time step (R0)
 * Step2: Obtain the torque acting on the grains at current time Step
 * Step3: Calculate the angular velocities around global axes
 * Step4: Calculate change in euler angles due to torque and corresponding rotation matrix(R1)
 * Step5: Calculate final rotation matrix, R = R1 * R0, determines the final position of any rotated
 * vector
 * Step6: Back-calculate the euler angles from the final rotation matrix
 * Step7: Ensure euler angles comply with Bunge definitions
 */
class EulerAngleUpdater : public EulerAngleProvider
{
public:
  static InputParameters validParams();

  EulerAngleUpdater(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual const EulerAngles & getEulerAngles(unsigned int) const override;
  virtual const EulerAngles & getEulerAnglesOld(unsigned int) const;
  virtual unsigned int getGrainNum() const override;

protected:
  const GrainTrackerInterface & _grain_tracker;
  const EulerAngleProvider & _euler;
  const GrainForceAndTorqueInterface & _grain_torque;
  const VectorPostprocessorValue & _grain_volumes;

  const Real _mr;
  /// Whether this is the first time updating angles, in which case the initial euler angle provider should be used
  bool & _first_time;
  /// Whether the simulation has recovered once. This only serves to prevent using the old angles in initialize()
  /// as problem.converged() returns false on the very first initialize() call after recovering
  bool _first_time_recovered;
  /// Used to determine whether a timestep is being repeated
  int & _t_step_old;

  /// Current set of Euler angles (one per grain), updated on initialize()
  std::vector<EulerAngles> & _angles;
  /// Previous set of Euler angles, used when the time step failed to reset the angles (pre-update)
  std::vector<EulerAngles> & _angles_old;
};
