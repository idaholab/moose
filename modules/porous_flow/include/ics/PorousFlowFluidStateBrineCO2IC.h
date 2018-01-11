/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDSTATEBRINECO2IC_H
#define POROUSFLOWFLUIDSTATEBRINECO2IC_H

#include "PorousFlowFluidStateICBase.h"

class PorousFlowBrineCO2;
class PorousFlowFluidStateBrineCO2IC;

template <>
InputParameters validParams<PorousFlowFluidStateBrineCO2IC>();

/**
 * PorousFlowFluidStateBrineCO2IC calculates an initial value for
 * the total mass fraction of a component summed over all
 * phases, z, for brine and CO2
 */
class PorousFlowFluidStateBrineCO2IC : public PorousFlowFluidStateICBase
{
public:
  PorousFlowFluidStateBrineCO2IC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

private:
  /// NaCl mass fraction (kg/kg)
  const VariableValue & _xnacl;
  /// FluidState UserObject
  const PorousFlowBrineCO2 & _fs_uo;
};

#endif // POROUSFLOWFLUIDSTATEBRINECO2IC_H
