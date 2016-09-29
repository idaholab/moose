/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINADVECTIONAUX_H
#define GRAINADVECTIONAUX_H

#include "AuxKernel.h"
#include "GrainTrackerInterface.h"
#include "GrainForceAndTorqueInterface.h"
// #include "DerivativeMaterialInterface.h"

//Forward Declarations
class GrainAdvectionAux;

template<>
InputParameters validParams<GrainAdvectionAux>();

/**
 * This Material calculates the advection velocity, it's divergence and
 * derivatives acting on a particle/grain
 */
class GrainAdvectionAux : public AuxKernel
{
public:
  GrainAdvectionAux(const InputParameters & parameters);

protected:
  virtual void precalculateValue();
  /// obtain total no. of grains from GrainTracker
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
  Real _mt;

  /// constant value corresponding to grain rotation
  Real _mr;

  RealGradient _velocity_advection;
  MooseEnum _component;
};

#endif //GrainAdvectionAuxAUX_H
