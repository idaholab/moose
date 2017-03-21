/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MULTICOMPONENTFLUIDPROPERTIESMATERIALPT_H
#define MULTICOMPONENTFLUIDPROPERTIESMATERIALPT_H

#include "Material.h"
#include "MultiComponentFluidPropertiesPT.h"

class MultiComponentFluidPropertiesMaterialPT;

template <>
InputParameters validParams<MultiComponentFluidPropertiesMaterialPT>();

/**
 * Material for testing the BrineFluidProperties UserObject.
 * Note: This material is for testing purposes only and shouldn't be
 * used for actual simulations. It provides access to internal functions
 * as well as the functions provided by the base FluidProperties class
 */
class MultiComponentFluidPropertiesMaterialPT : public Material
{
public:
  MultiComponentFluidPropertiesMaterialPT(const InputParameters & parameters);
  virtual ~MultiComponentFluidPropertiesMaterialPT();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;
  /// Mass fraction (-)
  const VariableValue & _xnacl;
  /// Density (kg/m^3)
  MaterialProperty<Real> & _rho;
  /// Enthalpy (kJ/kg)
  MaterialProperty<Real> & _h;
  /// Isobaric heat capacity (kJ/kg/K)
  MaterialProperty<Real> & _cp;
  /// Internal energy (kJ/kg)
  MaterialProperty<Real> & _e;

  // Multicomponent fluid properties UserObject
  const MultiComponentFluidPropertiesPT & _fp;
};

#endif /* MULTICOMPONENTFLUIDPROPERTIESMATERIALPT_H */
