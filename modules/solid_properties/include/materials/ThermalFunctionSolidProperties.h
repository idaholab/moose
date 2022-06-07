//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THERMALFUNCTIONMATERIAL_H
#define THERMALFUNCTIONMATERIAL_H

#include "ThermalSolidPropertiesMaterial.h"
#include "Function.h"

class ThermalFunctionSolidProperties;

template <>
InputParameters validParams<ThermalFunctionSolidProperties>();

/**
 * Thermal material properties as a function of temperature from function
 * inputs. These functions are parameterized as a function of time such that
 * 't' corresponds to temperature, not time. This user object can also be
 * used to specify constant properties.
 */
class ThermalFunctionSolidProperties : public ThermalSolidPropertiesMaterial
{
public:
  ThermalFunctionSolidProperties(const InputParameters & parameters);

  /**
   * Solid name
   * @return "thermal_function"
   */
  virtual const std::string & solidName() const override;

  /// Isobaric specific heat capacity
  virtual void computeIsobaricSpecificHeat() override;

  /// Isobaric specific heat capacity derivatives
  virtual void computeIsobaricSpecificHeatDerivatives() override;

  /// Thermal conductivity
  virtual void computeThermalConductivity() override;

  /// Thermal conductivity derivatives
  virtual void computeThermalConductivityDerivatives() override;

  /// Density
  virtual void computeDensity() override;

  /// Density derivatives
  virtual void computeDensityDerivatives() override;

protected:
  /// Function providing the thermal conductivity as a function of temperature
  const Function & _k_function;

  /// Function providing the isobaric specific heat as a function of temperature
  const Function & _cp_function;

  /// Function providing the density as a function of temperature
  const Function & _rho_function;

private:
  /// The solid name
  static const std::string _name;
};

#endif /* THERMALFUNCTIONMATERIAL_H */
