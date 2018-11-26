//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWFLUIDSTATEWATERNCG_H
#define POROUSFLOWFLUIDSTATEWATERNCG_H

#include "PorousFlowFluidState.h"

class PorousFlowFluidStateWaterNCG;

template <>
InputParameters validParams<PorousFlowFluidStateWaterNCG>();

/**
 * Fluid state class for water and a non-condensable gas. Calculates the solubility
 * of the gas phase in the water using Henry's law, and provides density, viscosity
 * and mass fractions for use in Kernels.
 */
class PorousFlowFluidStateWaterNCG : public PorousFlowFluidState
{
public:
  PorousFlowFluidStateWaterNCG(const InputParameters & parameters);

protected:
};

#endif // POROUSFLOWFLUIDSTATEWATERNCG_H
