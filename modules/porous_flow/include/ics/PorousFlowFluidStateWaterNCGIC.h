/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
