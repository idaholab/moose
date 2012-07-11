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

  const Real & _my_thermal_conductivity;
  Real _my_specific_heat;

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _thermal_conductivity_dT;
  MaterialProperty<Real> & _specific_heat;
};

#endif //HEATCONDUCTIONMATERIAL_H
