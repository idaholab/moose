//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THERMALSOLIDPROPERTIESMATERIAL_H
#define THERMALSOLIDPROPERTIESMATERIAL_H

#include "SolidPropertiesMaterial.h"
#include "ThermalSolidProperties.h"

class ThermalSolidPropertiesMaterial;

template <>
InputParameters validParams<ThermalSolidPropertiesMaterial>();

/**
 * Base class for defining thermal solid property materials as a function of
 * temperature.
 */
class ThermalSolidPropertiesMaterial : public SolidPropertiesMaterial
{
public:
  ThermalSolidPropertiesMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Temperature
  const VariableValue & _temperature;

  /// Isobaric specific heat capacity
  MaterialProperty<Real> & _cp;

  /// Thermal conductivity
  MaterialProperty<Real> & _k;

  /// Density
  MaterialProperty<Real> & _rho;

  /// Solid properties UserObject
  const ThermalSolidProperties & _sp;
};

#endif /* THERMALSOLIDPROPERTIESMATERIAL_H */
