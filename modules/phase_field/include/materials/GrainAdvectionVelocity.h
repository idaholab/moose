/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINADVECTIONVELOCITY_H
#define GRAINADVECTIONVELOCITY_H

#include "Material.h"
#include "GrainTrackerInterface.h"
#include "GrainForceAndTorqueInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class GrainAdvectionVelocity;

template <>
InputParameters validParams<GrainAdvectionVelocity>();

/**
 * This Material calculates the advection velocity, it's divergence and
 * derivatives acting on a particle/grain
 */
class GrainAdvectionVelocity : public DerivativeMaterialInterface<Material>
{
public:
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
  std::string _base_name;

  /// Material storing advection velocities of grains
  MaterialProperty<std::vector<RealGradient>> & _velocity_advection;
};

#endif // GRAINADVECTIONVELOCITY_H
