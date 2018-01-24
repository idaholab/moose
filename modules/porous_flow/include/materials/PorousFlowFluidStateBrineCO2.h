/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDSTATEBRINECO2_H
#define POROUSFLOWFLUIDSTATEBRINECO2_H

#include "PorousFlowFluidStateFlashBase.h"

class PorousFlowBrineCO2;
class PorousFlowFluidStateBrineCO2;

template <>
InputParameters validParams<PorousFlowFluidStateBrineCO2>();

/**
 * Fluid state class for brine and CO2. Includes mutual solubility of CO2 and
 * brine using model of Spycher, Pruess and Ennis-King, CO2-H2O mixtures in the
 * geological sequestration of CO2. I. Assessment and calculation of mutual
 * solubilities from 12 to 100C and up to 600 bar, Geochimica et Cosmochimica Acta,
 * 67, 3015-3031 (2003), and
 * Spycher and Pruess, CO2-H2O mixtures in the geological sequestration of CO2. II.
 * Partitioning in chloride brine at 12-100C and up to 600 bar, Geochimica et
 * Cosmochimica Acta, 69, 3309-3320 (2005)
 */
class PorousFlowFluidStateBrineCO2 : public PorousFlowFluidStateFlashBase
{
public:
  PorousFlowFluidStateBrineCO2(const InputParameters & parameters);

protected:
  virtual void thermophysicalProperties() override;

  /// Salt mass fraction (kg/kg)
  const VariableValue & _xnacl;
  /// FluidState UserObject
  const PorousFlowBrineCO2 & _fs_uo;
};

#endif // POROUSFLOWFLUIDSTATEBRINECO2_H
