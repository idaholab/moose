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
 * A material that computes 304L surface stainless steel properties relevant to doing laser
 * welding modeling. The functional form of these properties come from:
 *
 * techreport{noble2007use,
 *   title={Use of Aria to simulate laser weld pool dynamics for neutron generator production.},
 *   author={Noble, David R and Notz, Patrick K and Martinez, Mario J and Kraynik, Andrew Michael},
 *   year={2007},
 *   institution={Sandia National Laboratories (SNL), Albuquerque, NM, and Livermore, CA}}
 */
class AriaLaserWeld304LStainlessSteelBoundary : public Material
{
public:
  static InputParameters validParams();

  AriaLaserWeld304LStainlessSteelBoundary(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Real _ap0;
  const Real _ap1;
  const Real _ap2;
  const Real _ap3;
  const Real _bp0;
  const Real _bp1;
  const Real _bp2;
  const Real _bp3;
  const Real _Tb;
  const Real _Tbound1;
  const Real _Tbound2;

  const ADVariableValue & _temperature;
  const ADVariableGradient & _grad_temperature;

  ADMaterialProperty<Real> & _rc_pressure;

  const Real _alpha;
  const Real _sigma0;
  const Real _T0;
  ADMaterialProperty<Real> & _surface_tension;
  ADMaterialProperty<RealVectorValue> & _grad_surface_tension;
  const MooseArray<ADPoint> & _ad_normals;
  const MooseArray<ADReal> & _ad_curvatures;
  ADMaterialProperty<RealVectorValue> & _surface_term_curvature;
  ADMaterialProperty<RealVectorValue> & _surface_term_gradient1;
  ADMaterialProperty<RealVectorValue> & _surface_term_gradient2;
};
