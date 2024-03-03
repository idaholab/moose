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

/**
 * This material computes the wave speed for dynamic simulations using
 * the Young's modulus (or equivalent metric) and the density.
 * This object can be used, for example, in contact algorithms for
 * explicit dynamics.
 */
class WaveSpeed : public Material
{
public:
  static InputParameters validParams();

  WaveSpeed(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  /// The wave speed material generated here
  MaterialProperty<Real> & _wave_speed;

  /// Density of the material
  const MaterialProperty<Real> & _material_density;

  /// Effective stiffness of element: function of material properties and element size
  const MaterialProperty<Real> & _effective_stiffness;
};
