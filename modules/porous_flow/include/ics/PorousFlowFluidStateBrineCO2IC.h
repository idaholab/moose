//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
