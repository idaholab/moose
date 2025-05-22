//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

class SinglePhaseFluidProperties;

/**
 * IC for junction variables in VolumeJunction1Phase.
 */
class VolumeJunction1PhaseIC : public InitialCondition
{
public:
  static InputParameters validParams();

  VolumeJunction1PhaseIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Quantity type
  enum class Quantity
  {
    RHOV,
    RHOUV,
    RHOVV,
    RHOWV,
    RHOEV,
    P,
    T,
    VEL
  };
  /// Which quantity to compute
  const Quantity _quantity;

  /// Pressure
  const Function & _p_fn;
  /// Temperature
  const Function & _T_fn;
  /// X velocity
  const Function & _vel_x_fn;
  /// Y velocity
  const Function & _vel_y_fn;
  /// Z velocity
  const Function & _vel_z_fn;

  /// Volume of the junction
  const Real _volume;
  /// Spatial position of center of the junction
  const Point & _position;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
