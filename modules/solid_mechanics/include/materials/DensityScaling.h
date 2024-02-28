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
#include "RankTwoTensor.h"

/**
 * This material computes the mass required to fulfill a prescribed critical time step.
 * Adding mass to elements can affect the dynamics of the system. For this reason, this
 * should only be done when the user knows that such additions will not affect the numerical
 * results. One such example is the existence of a number of very small elements in the
 * mesh, whose dynamics are not critical to the simulation key metrics.
 */
class DensityScaling : public Material
{
public:
  static InputParameters validParams();

  DensityScaling(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  /// User-prescribed desired time step. Mass will be added until fulfilled.
  const Real _desired_time_step;

  /// The stress tensor
  MaterialProperty<Real> & _density_scaling;

  /// Density of the material
  const MaterialProperty<Real> & _material_density;

  /// Effective stiffness of element: function of material properties and element size
  const MaterialProperty<Real> & _effective_stiffness;

  /// User defined factor to be multiplied to the critical time step
  const Real & _factor;
};
