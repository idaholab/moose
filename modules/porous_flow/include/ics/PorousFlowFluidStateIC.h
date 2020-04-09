//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

class PorousFlowDictator;
class PorousFlowFluidStateMultiComponentBase;

/**
 * PorousFlowFluidStateIC calculates an initial value for
 * the total mass fraction of a component summed over all
 * phases, z.
 */
class PorousFlowFluidStateIC : public InitialCondition
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Gas porepressure (Pa)
  const VariableValue & _gas_porepressure;
  /// Fluid temperature (C or K)
  const VariableValue & _temperature;
  /// NaCl mass fraction (kg/kg)
  const VariableValue & _Xnacl;
  /// Gas saturation (-)
  const VariableValue & _saturation;
  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _T_c2k;
  /// The PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;
  /// FluidState UserObject
  const PorousFlowFluidStateMultiComponentBase & _fs;
};
