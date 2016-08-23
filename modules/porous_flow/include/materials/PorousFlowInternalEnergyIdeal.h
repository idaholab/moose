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

template<>
InputParameters validParams<PorousFlowInternalEnergyIdeal>();

/**
 * This material computes internal energy (J/kg) for a fluid assuming
 * that its specific heat capacity at constant volume is constant.
 */
class PorousFlowInternalEnergyIdeal : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowInternalEnergyIdeal(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();

  virtual void computeQpProperties();

  /// Specific heat capacity at constant volume (J/kg/K)
  const Real _cv;

  /// Fluid phase internal_energy at the nodes
  MaterialProperty<Real> & _internal_energy_nodal;

  /// Old fluid phase internal_energy at the nodes
  MaterialProperty<Real> & _internal_energy_nodal_old;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _dinternal_energy_nodal_dp;

  /// Derivative of fluid internal_energy wrt temperature at the nodes
  MaterialProperty<Real> & _dinternal_energy_nodal_dt;

  /// Fluid phase internal_energy at the qps
  MaterialProperty<Real> & _internal_energy_qp;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the qps
  MaterialProperty<Real> & _dinternal_energy_qp_dp;

  /// Derivative of fluid internal_energy wrt temperature at the qps
  MaterialProperty<Real> & _dinternal_energy_qp_dt;
};

#endif // POROUSFLOWINTERNALENERGYIDEAL_H
