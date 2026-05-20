//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes several quantities for VolumeJunction1Phase.
 */
class VolumeJunction1PhaseFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  VolumeJunction1PhaseFunctorMaterial(const InputParameters & parameters);

protected:
  /// Adds pressure functor material property
  void addPressureFunctorProperty();
  /// Adds temperature functor material property
  void addTemperatureFunctorProperty();
  /// Adds velocity component functor material property
  void addVelocityComponentFunctorProperty(const std::string & property,
                                           const Moose::Functor<ADReal> & rhouV_i);

  /// Computes specific volume and specific internal energy
  template <typename SpaceArg, typename StateArg>
  void computeSpecificVolumeAndInternalEnergy(const SpaceArg & r,
                                              const StateArg & t,
                                              Real & v,
                                              Real & e) const;

  /// rho*V
  const Moose::Functor<ADReal> & _rhoV;
  /// rho*u*V
  const Moose::Functor<ADReal> & _rhouV;
  /// rho*v*V
  const Moose::Functor<ADReal> & _rhovV;
  /// rho*w*V
  const Moose::Functor<ADReal> & _rhowV;
  /// rho*E*V
  const Moose::Functor<ADReal> & _rhoEV;
  /// Junction volume
  const Real _V;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
