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

// Forward Declarations
class GrainForceAndTorqueInterface;

/**
 *  GrainForcesPostprocessor is a type of VectorPostprocessor that outputs the
 *  force and torque values calculated in UserObjects.
 */
class GrainForcesPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  GrainForcesPostprocessor(const InputParameters & parameters);

  virtual ~GrainForcesPostprocessor() {}
  virtual void initialize();
  virtual void execute();

protected:
  /// The VectorPostprocessorValue object where the results are stored
  VectorPostprocessorValue & _grain_force_torque_vector;

  /// UserobjectInterface for getting force and torque values from UserObjects
  const GrainForceAndTorqueInterface & _grain_force_torque;
  /// Extracting forces from Userobject
  const std::vector<RealGradient> & _grain_forces;
  /// Extracting torques from Userobject
  const std::vector<RealGradient> & _grain_torques;
  /// total no. of grains
  unsigned int _grain_num;
};
