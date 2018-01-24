//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWFLUIDSTATEWATERNCGIC_H
#define POROUSFLOWFLUIDSTATEWATERNCGIC_H

#include "PorousFlowFluidStateICBase.h"

class PorousFlowWaterNCG;
class PorousFlowFluidStateWaterNCGIC;

template <>
InputParameters validParams<PorousFlowFluidStateWaterNCGIC>();

/**
 * PorousFlowFluidStateWaterNCGIC calculates an initial value for
 * the total mass fraction of a component summed over all
 * phases, z, for water and non-condensable gas
 */
class PorousFlowFluidStateWaterNCGIC : public PorousFlowFluidStateICBase
{
public:
  PorousFlowFluidStateWaterNCGIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

private:
  /// FluidState UserObject
  const PorousFlowWaterNCG & _fs_uo;
};

#endif // POROUSFLOWFLUIDSTATEWATERNCGIC_H
