/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
