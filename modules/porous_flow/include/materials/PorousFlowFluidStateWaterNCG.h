/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDSTATEWATERNCG_H
#define POROUSFLOWFLUIDSTATEWATERNCG_H

#include "PorousFlowFluidStateFlashBase.h"
#include "Water97FluidProperties.h"

class PorousFlowWaterNCG;
class PorousFlowFluidStateWaterNCG;

template <>
InputParameters validParams<PorousFlowFluidStateWaterNCG>();

/**
 * Fluid state class for water and a non-condensable gas. Calculates the solubility
 * of the gas phase in the water using Henry's law, and provides density, viscosity
 * and mass fractions for use in Kernels.
 */
class PorousFlowFluidStateWaterNCG : public PorousFlowFluidStateFlashBase
{
public:
  PorousFlowFluidStateWaterNCG(const InputParameters & parameters);

protected:
  virtual void thermophysicalProperties();

  /// FluidState UserObject
  const PorousFlowWaterNCG & _fs_uo;
};

#endif // POROUSFLOWFLUIDSTATEWATERNCG_H
