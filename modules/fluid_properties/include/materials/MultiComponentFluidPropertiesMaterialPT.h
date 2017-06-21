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
 * Material for calculating fluid properties for a fluid comprised of two components:
 * the solute (eg, NaCl), and the solution (eg, water). This material uses the
 * pressure - temperature formulation.
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
  /// Mass fraction of solute (-)
  const VariableValue & _xmass;
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
