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
 * A material that computes 316L surface stainless steel boundary properties relevant to doing laser
 * welding modeling (recoil pressure, surface tension).
 */
class LaserWeld316LStainlessSteelBoundary : public Material
{
public:
  static InputParameters validParams();

  LaserWeld316LStainlessSteelBoundary(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /**
   * The vapor recoil pressure is taken from:
   * Chen, Xuehui, et al. "Numerical analysis of double track formation for selective
   * laser melting of 316L stainless steel." Applied Physics A 127 (2021): 1-13.
   */
  const Real _P0;
  const Real _L_v;
  const Real _M;
  const Real _T_v;
  const Real _R;

  /**
   * The surface tension is taken from:
   * Pichler, Peter, et al. "Surface tension and thermal conductivity of NIST SRM 1155a
   * (AISI 316L stainless steel)."
   * International Journal of Thermophysics 43.5 (2022): 66.
   */
  const Real _c_gamma0;
  const Real _c_gamma1;

  /// The liquidus temperature
  const Real _Tl;

  /// Declaring the material properties
  ADMaterialProperty<Real> & _rc_pressure;
  ADMaterialProperty<Real> & _surface_tension;
  ADMaterialProperty<RealVectorValue> & _grad_surface_tension;
  ADMaterialProperty<RealVectorValue> & _surface_term_curvature;
  ADMaterialProperty<RealVectorValue> & _surface_term_gradient1;
  ADMaterialProperty<RealVectorValue> & _surface_term_gradient2;
  const MooseArray<ADPoint> & _ad_normals;
  const MooseArray<ADReal> & _ad_curvatures;

  /// We need to know the temperature and the gradient of the temperature
  const ADVariableValue & _temperature;
  const ADVariableGradient & _grad_temperature;
};
