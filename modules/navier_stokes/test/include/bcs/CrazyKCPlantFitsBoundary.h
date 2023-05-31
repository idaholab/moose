/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

#pragma once

#include "ADMaterial.h"

/**
 * A material that couples a material property
 */
class CrazyKCPlantFitsBoundary : public ADMaterial
{
public:
  static InputParameters validParams();

  CrazyKCPlantFitsBoundary(const InputParameters & parameters);

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

  const Real _length_units_per_meter;
  const Real _temperature_units_per_kelvin;
  const Real _mass_units_per_kilogram;
  const Real _time_units_per_second;
};
