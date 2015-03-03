/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATCONDUCTIONMATERIAL_H
#define HEATCONDUCTIONMATERIAL_H

#include "Material.h"


//Forward Declarations
class HeatConductionMaterial;
class Function;

template<>
InputParameters validParams<HeatConductionMaterial>();

/**
 * Simple material with constant properties.
 */
class HeatConductionMaterial : public Material
{
public:
  HeatConductionMaterial(const std::string & name,
                         InputParameters parameters);

protected:
  virtual void computeProperties();

  const bool _has_temp;
  VariableValue & _temperature;

  const Real _my_thermal_conductivity;
  const Real _my_specific_heat;

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _thermal_conductivity_dT;
  Function * const _thermal_conductivity_temperature_function;

  MaterialProperty<Real> & _specific_heat;
  Function * const _specific_heat_temperature_function;

};

#endif //HEATCONDUCTIONMATERIAL_H
