//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes fluid properties using (pressure, specific enthalpy) formulation
 */
class FluidPropertiesMaterialPH : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesMaterialPH(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterialPH();

protected:
  void computeQpProperties() override;

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Specific enthalpy (J/kg)
  const VariableValue & _h;

  /// Temperature (K)
  MaterialProperty<Real> & _T;
  /// Specific entropy (J/kg/K)
  MaterialProperty<Real> & _s;

  /// Fluid properties UserObject
  const SinglePhaseFluidProperties & _fp;
};
