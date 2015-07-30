#ifndef CONSTGRAINADVECTIONVELOCITY_H
#define CONSTGRAINADVECTIONVELOCITY_H

#include "Material.h"
#include "ComputeGrainCenterUserObject.h"
#include "ConstantGrainForceAndTorque.h"


//Forward Declarations
class ConstGrainAdvectionVelocity;

template<>
InputParameters validParams<ConstGrainAdvectionVelocity>();

/**
 * This Material calculates the advection velocity and it's divergence acting on a particle/grain
 */
class ConstGrainAdvectionVelocity : public Material
{
public:
  ConstGrainAdvectionVelocity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// getting userobject for calculating grain centers and volumes
  const ComputeGrainCenterUserObject & _grain_data;
  const std::vector<Real> & _grain_volumes;
  const std::vector<Point> & _grain_centers;

  /// getting userobject for calculating grain forces and torques
  const ConstantGrainForceAndTorque & _grain_force_torque;
  const std::vector<RealGradient> & _grain_forces;
  const std::vector<RealGradient> & _grain_torques;
  const std::vector<RealGradient> & _grain_force_derivatives;
  const std::vector<RealGradient> & _grain_torque_derivatives;

private:
  /// equilibrium density at the grain boundaries
  Real _mt;

  /// thresold value for identifying grain boundaries
  Real _mr;

  unsigned int _ncrys;
  std::vector<VariableValue *> _vals;
  std::vector<VariableGradient *> _grad_vals;

  /// Material storing advection velocities of grains
  MaterialProperty<std::vector<RealGradient> > & _velocity_advection;

  /// Material storing divergence of advection velocities of grains
  MaterialProperty<std::vector<Real> > & _div_velocity_advection;

  /// Material storing advection velocities of grains
  MaterialProperty<std::vector<RealGradient> > & _velocity_advection_derivative;

  /// Material storing divergence of advection velocities of grains
  MaterialProperty<std::vector<Real> > & _div_velocity_advection_derivative;
};

#endif //CONSTGRAINADVECTIONVELOCITY_H
