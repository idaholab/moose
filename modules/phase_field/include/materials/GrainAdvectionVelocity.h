//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "GrainTrackerInterface.h"
#include "GrainForceAndTorqueInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * This Material calculates the advection velocity, it's divergence and
 * derivatives acting on a particle/grain
 */
class GrainAdvectionVelocity : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  GrainAdvectionVelocity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// getting userobject for calculating grain centers and volumes
  const GrainTrackerInterface & _grain_tracker;

  /// getting userobject for calculating grain forces and torques
  const GrainForceAndTorqueInterface & _grain_force_torque;

  /// The grain volumes
  const VectorPostprocessorValue & _grain_volumes;

  const std::vector<RealGradient> & _grain_forces;
  const std::vector<RealGradient> & _grain_torques;

private:
  /// constant value corresponding to grain translation
  const Real _mt;
  /// constant value corresponding to grain rotation
  const Real _mr;

  const unsigned int _op_num;

  /// type of force density material
  const std::string _base_name;

  /// Material storing advection velocities of grains
  MaterialProperty<std::vector<RealGradient>> & _velocity_advection;
};
