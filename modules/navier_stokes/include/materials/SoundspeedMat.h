//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ADMaterial.h"

class SoundspeedMat;
class SinglePhaseFluidProperties;

declareADValidParams(SoundspeedMat);

/**
 * Computes the speed of sound from other Navier-Stokes materials
 */
class SoundspeedMat : public ADMaterial
{
public:
  SoundspeedMat(const InputParameters & parameters);

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
