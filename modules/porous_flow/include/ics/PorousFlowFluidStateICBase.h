//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWFLUIDSTATEICBASE_H
#define POROUSFLOWFLUIDSTATEICBASE_H

#include "InitialCondition.h"

class PorousFlowDictator;
class PorousFlowFluidStateICBase;

template <>
InputParameters validParams<PorousFlowFluidStateICBase>();

/**
 * PorousFlowFluidStateIC calculates an initial value for
 * the total mass fraction of a component summed over all
 * phases, z.
 */
class PorousFlowFluidStateICBase : public InitialCondition
{
public:
  PorousFlowFluidStateICBase(const InputParameters & parameters);

protected:
  /// Gas porepressure (Pa)
  const VariableValue & _gas_porepressure;
  /// Fluid temperature (C or K)
  const VariableValue & _temperature;
  /// Gas saturation (-)
  const VariableValue & _saturation;
  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _T_c2k;
  /// The PorousFlow dictator UserObject
  const PorousFlowDictator & _dictator;
};

#endif // POROUSFLOWFLUIDSTATEICBASE_H
