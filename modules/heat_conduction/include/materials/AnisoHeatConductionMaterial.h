//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ANISOHEATCONDUCTIONMATERIAL_H
#define ANISOHEATCONDUCTIONMATERIAL_H

#include "Material.h"

// Forward Declarations
class Function;

/**
 * Simple material with constant properties.
 */
class AnisoHeatConductionMaterial : public Material
{
public:
  AnisoHeatConductionMaterial(const InputParameters & parameters);

protected:
  virtual void computeProperties();

  const bool _has_temp;
  const VariableValue & _temperature;

  const Real _my_thermal_conductivity_x;
  const Real _my_thermal_conductivity_y;
  const Real _my_thermal_conductivity_z;
  const PostprocessorValue * const _thermal_conductivity_x_pp;
  const PostprocessorValue * const _thermal_conductivity_y_pp;
  const PostprocessorValue * const _thermal_conductivity_z_pp;
  const Real _my_specific_heat;

  MaterialProperty<Real> * const _thermal_conductivity_x;
  MaterialProperty<Real> * const _thermal_conductivity_x_dT;
  MaterialProperty<Real> * const _thermal_conductivity_y;
  MaterialProperty<Real> * const _thermal_conductivity_y_dT;
  MaterialProperty<Real> * const _thermal_conductivity_z;
  MaterialProperty<Real> * const _thermal_conductivity_z_dT;

  MaterialProperty<Real> & _specific_heat;
  Function * const _specific_heat_temperature_function;
};

template <>
InputParameters validParams<AnisoHeatConductionMaterial>();

#endif // ANISOHEATCONDUCTIONMATERIAL_H
