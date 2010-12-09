#ifndef HEATCONDUCTIONMATERIAL_H
#define HEATCONDUCTIONMATERIAL_H

#include "Material.h"


//Forward Declarations
class HeatConductionMaterial;

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

private:

  bool _has_temp;
  VariableValue & _temp;

  Real _my_thermal_conductivity;
  Real _my_specific_heat;
  Real _my_density;

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _specific_heat;
  MaterialProperty<Real> & _density;
};

#endif //HEATCONDUCTIONMATERIAL_H
