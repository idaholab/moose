/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
