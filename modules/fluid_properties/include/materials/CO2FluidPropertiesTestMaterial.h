/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CO2FLUIDPROPERTIESTESTMATERIAL_H
#define CO2FLUIDPROPERTIESTESTMATERIAL_H

#include "CO2FluidProperties.h"
#include "Material.h"

class CO2FluidPropertiesTestMaterial;

template<>
InputParameters validParams<CO2FluidPropertiesTestMaterial>();

/**
 * Material for testing the CO2FluidProperties.
 * Note: This material is for testing purposes only and shouldn't be
 * used for actual simulations. It provides access to internal functions
 * as well as the functions provided by the base FluidProperties class
 */
class CO2FluidPropertiesTestMaterial : public Material
{
public:
  CO2FluidPropertiesTestMaterial(const InputParameters & parameters);
  virtual ~CO2FluidPropertiesTestMaterial();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;
  /// Sublimation pressure (Pa)
  MaterialProperty<Real> & _psub;
  /// Melting pressure (Pa)
  MaterialProperty<Real> & _pmelt;
  /// Vapor pressure (Pa)
  MaterialProperty<Real> & _pvap;
  /// Saturated vapor density (kg/m^3)
  MaterialProperty<Real> & _rhovap;
  /// Saturated liquid density (kg/m^3)
  MaterialProperty<Real> & _rhosat;
  /// Partial density (kg/m^3)
  MaterialProperty<Real> & _rhopartial;

  /// CO2 Fluid properties UserObject
  const CO2FluidProperties & _fp;
  /// Boolean flag to calculate sublimation pressure
  const bool _sublimation;
  /// Boolean flag to calculate melting pressure
  const bool _melting;
  /// Boolean flag to calculate vapor pressure
  const bool _vapor;
};

#endif /* CO2FLUIDPROPERTIESMATERIAL_H */
