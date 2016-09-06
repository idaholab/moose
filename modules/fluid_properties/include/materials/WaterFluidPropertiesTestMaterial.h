/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef WATERFLUIDPROPERTIESTESTMATERIAL_H
#define WATERFLUIDPROPERTIESTESTMATERIAL_H

#include "Material.h"
#include "Water97FluidProperties.h"

class WaterFluidPropertiesTestMaterial;

template<>
InputParameters validParams<WaterFluidPropertiesTestMaterial>();

/**
 * Material for testing the Water97FluidProperties.
 * Note: This material is for testing purposes only and shouldn't be
 * used for actual simulations. It provides access to internal functions
 * as well as the functions provided by the base FluidProperties class
 */
class WaterFluidPropertiesTestMaterial : public Material
{
public:
  WaterFluidPropertiesTestMaterial(const InputParameters & parameters);
  virtual ~WaterFluidPropertiesTestMaterial();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;
  /// Density (kg/m^3) - optional variable viscosity calculations
  const VariableValue & _density;
  /// Boolean to control calculation of saturation properties
  const bool _region4;
  /// Boolean to control calculation of b23p and b23T
  const bool _b23;
  /// Boolean to control calculation of viscosity using given density
  const bool _viscosity;
  /// Density
  MaterialProperty<Real> & _rho;
  /// Viscosity
  MaterialProperty<Real> & _mu;
  /// Internal energy
  MaterialProperty<Real> & _e;
  /// Enthalpy
  MaterialProperty<Real> & _h;
  /// Entropy
  MaterialProperty<Real> & _s;
  /// Isobaric specific heat capacity
  MaterialProperty<Real> & _cp;
  /// Isochoric specific heat capacity
  MaterialProperty<Real> & _cv;
  /// Speed of sound
  MaterialProperty<Real> & _c;
  /// Thermal conductivity (W/m/K)
  MaterialProperty<Real> & _k;
  /// Saturation pressure
  MaterialProperty<Real> & _psat;
  /// Saturation temperature
  MaterialProperty<Real> & _Tsat;
  /// Pressure along boundary between regions 2 and 3
  MaterialProperty<Real> & _b23p;
  /// Temperature along boundary between regions 2 and 3
  MaterialProperty<Real> & _b23T;
  /// Fluid properties UserObject
  const Water97FluidProperties & _fp;
};

#endif /* WATERFLUIDPROPERTIESTESTMATERIAL_H */
