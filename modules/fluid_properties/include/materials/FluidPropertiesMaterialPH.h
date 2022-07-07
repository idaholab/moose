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
 * Computes fluid properties using (pressure, temperature) formulation
 */
class FluidPropertiesMaterialPH : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesMaterialPH(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterialPH();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Specific enthalpy (kJ/kg)
  const VariableValue & _h;
  /// Specific entropy (kJ/kg/K)
  MaterialProperty<Real> & _s;

  /// Fluid properties UserObject
  const SinglePhaseFluidProperties & _fp;
};
