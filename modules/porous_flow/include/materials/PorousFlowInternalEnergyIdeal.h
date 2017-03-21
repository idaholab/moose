/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWINTERNALENERGYIDEAL_H
#define POROUSFLOWINTERNALENERGYIDEAL_H

#include "PorousFlowFluidPropertiesBase.h"

class PorousFlowInternalEnergyIdeal;

template <>
InputParameters validParams<PorousFlowInternalEnergyIdeal>();

/**
 * This material computes internal energy (J/kg) at the quadpoints or nodes for a fluid assuming
 * that its specific heat capacity at constant volume is constant.
 */
class PorousFlowInternalEnergyIdeal : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowInternalEnergyIdeal(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Specific heat capacity at constant volume (J/kg/K)
  const Real _cv;

  /// Fluid phase internal_energy at the qps or nodes
  MaterialProperty<Real> & _internal_energy;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> & _dinternal_energy_dp;

  /// Derivative of fluid internal_energy wrt temperature at the qps or nodes
  MaterialProperty<Real> & _dinternal_energy_dt;
};

#endif // POROUSFLOWINTERNALENERGYIDEAL_H
