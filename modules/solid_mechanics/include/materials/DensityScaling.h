//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * This material computes a density that is scaled to enable stable explicit time stepping 
 * with steps larger than those governed by stability conditions in solid-mechanics problems.
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
  /// User-prescribed desired time step
  const Real _desired_time_step;

  /// whether to compute the additional density needed (_additive_contribution = true) or the density needed (_additive_contribution = false)
  const bool _additive_contribution;

  /// The scaled density
  MaterialProperty<Real> & _density_scaled;

  /// The true inertial density of the material
  const MaterialProperty<Real> & _material_density;

  /// Square root of effective stiffness of element
  const MaterialProperty<Real> & _sqrt_effective_stiffness;

  /// User defined factor to be multiplied to the critical time step
  const Real & _safety_factor;
};
