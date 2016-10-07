/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef BRINEFLUIDPROPERTIESTESTMATERIAL_H
#define BRINEFLUIDPROPERTIESTESTMATERIAL_H

#include "Material.h"
#include "BrineFluidProperties.h"

class BrineFluidPropertiesTestMaterial;

template<>
InputParameters validParams<BrineFluidPropertiesTestMaterial>();

/**
 * Material for testing the BrineFluidProperties UserObject.
 * Note: This material is for testing purposes only and shouldn't be
 * used for actual simulations. It provides access to internal functions
 * as well as the functions provided by the base FluidProperties class
 */
class BrineFluidPropertiesTestMaterial : public Material
{
public:
  BrineFluidPropertiesTestMaterial(const InputParameters & parameters);
  virtual ~BrineFluidPropertiesTestMaterial();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;
  /// NaCl mass fraction (-)
  const VariableValue & _xnacl;
  /// Density (kg/m^3)
  MaterialProperty<Real> & _rho;
  /// Viscosity (Pa.s)
  MaterialProperty<Real> & _mu;
  /// Enthalpy (kJ/kg)
  MaterialProperty<Real> & _h;
  /// Isobaric heat capacity (kJ/kg/K)
  MaterialProperty<Real> & _cp;
  /// Internal energy (kJ/kg)
  MaterialProperty<Real> & _e;
  /// Thermal conductivity (W/m/K)
  MaterialProperty<Real> & _k;
  /// Halite solubility (kg/kg)
  MaterialProperty<Real> & _solubility;
  /// Saturated vapor pressure (Pa)
  MaterialProperty<Real> & _pvap;
  /// Halite density (kg/m^3)
  MaterialProperty<Real> & _rho_halite;
  /// Halite isobaric heat capacity (kJ/kg/K)
  MaterialProperty<Real> & _cp_halite;
  /// Halite enthalpy (kJ/kg)
  MaterialProperty<Real> & _h_halite;
  /// Boolean to control calculation of saturated vapor pressure
  const bool _vapor;
  /// Fluid properties UserObject
  const BrineFluidProperties & _fp;
  // Water properties UserObject
  const SinglePhaseFluidPropertiesPT & _water_fp;
  // NaCl properties UserObject
  const SinglePhaseFluidPropertiesPT & _halite_fp;
};

#endif /* BRINEFLUIDPROPERTIESTESTMATERIAL_H */
