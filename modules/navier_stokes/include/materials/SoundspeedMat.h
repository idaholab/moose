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

class SinglePhaseFluidProperties;

/**
 * Computes the speed of sound from other Navier-Stokes material properties
 */
class SoundspeedMat : public Material
{
public:
  SoundspeedMat(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

  /// The speed of sound
  ADMaterialProperty<Real> & _sound_speed;

  /// pressure
  const ADMaterialProperty<Real> & _pressure;

  /// temperature
  const ADMaterialProperty<Real> & _temperature;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fluid;
};
