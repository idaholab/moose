//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "GrainTrackerInterface.h"
#include "GrainForceAndTorqueInterface.h"

// Forward Declarations

/**
 * Calculates the advection velocity of grain due to rigid body motion
 * Reports the components of the velocity on each element
 */
class GrainAdvectionAux : public AuxKernel
{
public:
  static InputParameters validParams();

  GrainAdvectionAux(const InputParameters & parameters);

protected:
  /// calculate the advection velocity
  virtual void precalculateValue();
  /// output the component of advection velocity
  virtual Real computeValue();

  /// getting userobject for calculating grain centers and volumes
  const GrainTrackerInterface & _grain_tracker;

  /// The grain volumes
  const VectorPostprocessorValue & _grain_volumes;

  /// getting userobject for calculating grain forces and torques
  const GrainForceAndTorqueInterface & _grain_force_torque;
  const std::vector<RealGradient> & _grain_forces;
  const std::vector<RealGradient> & _grain_torques;

private:
  /// constant value corresponding to grain translation
  const Real _mt;

  /// constant value corresponding to grain rotation
  const Real _mr;

  RealGradient _velocity_advection;
  MooseEnum _component;
};
