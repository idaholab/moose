/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINADVECTIONVELOCITY_H
#define GRAINADVECTIONVELOCITY_H

#include "Material.h"
#include "GrainForceAndTorqueInterface.h"
#include "DerivativeMaterialInterface.h"

//Forward Declarations
class GrainTrackerInterface;
class GrainAdvectionVelocity;

template<>
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
  const std::vector<RealGradient> & _grain_forces;
  const std::vector<RealGradient> & _grain_torques;
  const std::vector<RealGradient> & _grain_force_derivatives;
  const std::vector<RealGradient> & _grain_torque_derivatives;

private:
  /// constant value corresponding to grain translation
  Real _mt;

  /// constant value corresponding to grain rotation
  Real _mr;

  unsigned int _ncrys;
  std::vector<const VariableValue *> _vals;
  std::vector<const VariableGradient *> _grad_vals;
  VariableName _c_name;
  /// type of force density material
  std::string _base_name;

  /// Material storing advection velocities of grains
  MaterialProperty<std::vector<RealGradient> > & _velocity_advection;

  /// Material storing divergence of advection velocities of grains
  MaterialProperty<std::vector<Real> > & _div_velocity_advection;

  /// Material storing derivative of advection velocities of grains w r. t. c
  MaterialProperty<std::vector<RealGradient> > & _velocity_advection_derivative_c;
  /// Material storing derivative of divergence of advection velocities of grains w r. t. c
  MaterialProperty<std::vector<Real> > & _div_velocity_advection_derivative_c;
  /// Material storing derivative of advection velocities of grains w r. t. eta
  MaterialProperty<std::vector<RealGradient> > & _velocity_advection_derivative_eta;
};

#endif //GRAINADVECTIONVELOCITY_H
