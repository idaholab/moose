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
  const PostprocessorValue * const _my_thermal_conductivity_x;
  const PostprocessorValue * const _my_thermal_conductivity_y;
  const PostprocessorValue * const _my_thermal_conductivity_z;
  const Real _my_specific_heat;

  const bool _isotropic_thcond;

  MaterialProperty<Real> * const _thermal_conductivity;
  MaterialProperty<Real> * const _thermal_conductivity_dT;
  MaterialProperty<Real> * const _thermal_conductivity_x;
  MaterialProperty<Real> * const _thermal_conductivity_x_dT;
  MaterialProperty<Real> * const _thermal_conductivity_y;
  MaterialProperty<Real> * const _thermal_conductivity_y_dT;
  MaterialProperty<Real> * const _thermal_conductivity_z;
  MaterialProperty<Real> * const _thermal_conductivity_z_dT;
  Function * const _thermal_conductivity_temperature_function;

  MaterialProperty<Real> & _specific_heat;
  Function * const _specific_heat_temperature_function;

};

#endif //HEATCONDUCTIONMATERIAL_H
