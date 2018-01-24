//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FLUIDPROPERTIESMATERIALPT_H
#define FLUIDPROPERTIESMATERIALPT_H

#include "Material.h"
#include "SinglePhaseFluidPropertiesPT.h"

class FluidPropertiesMaterialPT;

template <>
InputParameters validParams<FluidPropertiesMaterialPT>();

/**
 * Computes fluid properties using (pressure, temperature) formulation
 */
class FluidPropertiesMaterialPT : public Material
{
public:
  FluidPropertiesMaterialPT(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterialPT();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;
  /// Density (kg/m^3)
  MaterialProperty<Real> & _rho;
  /// Viscosity (Pa.s)
  MaterialProperty<Real> & _mu;
  /// Isobaric specific heat capacity (kJ/kg/K)
  MaterialProperty<Real> & _cp;
  /// Isochoric specific heat capacity (kJ/kg/K)
  MaterialProperty<Real> & _cv;
  /// Thermal conductivity (W/m/K)
  MaterialProperty<Real> & _k;
  /// Specific enthalpy (kJ/kg)
  MaterialProperty<Real> & _h;
  /// Internal energy (kJ/kg)
  MaterialProperty<Real> & _e;
  /// Specific entropy (kJ/kg/K)
  MaterialProperty<Real> & _s;
  /// Speed of sound (m/s)
  MaterialProperty<Real> & _c;

  /// Fluid properties UserObject
  const SinglePhaseFluidPropertiesPT & _fp;
};

#endif /* FLUIDPROPERTIESMATERIALPT_H */
