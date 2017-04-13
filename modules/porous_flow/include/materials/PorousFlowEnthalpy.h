/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWENTHALPY_H
#define POROUSFLOWENTHALPY_H

#include "PorousFlowFluidPropertiesBase.h"

class PorousFlowEnthalpy;

template <>
InputParameters validParams<PorousFlowEnthalpy>();

/**
 * This material computes specific enthalpy (J/kg) at the quadpoints for a fluid assuming
 * using the relationship
 * specify_enthalpy = specific_internal_energy + pressure/density
 */
class PorousFlowEnthalpy : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowEnthalpy(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// enthalpy = internal_energy + porepressure / density * _pp_coeff
  const Real _pp_coeff;

  /// Fluid energy at the quadpoints or nodes
  const MaterialProperty<Real> & _energy;

  /// d(Fluid energy at the quadpoints or nodes)/d(porepressure at the quadpoints or nodes)
  const MaterialProperty<Real> & _denergy_dp;

  /// d(Fluid energy at the quadpoints or nodes)/d(temperature at the quadpoints or nodes)
  const MaterialProperty<Real> & _denergy_dt;

  /// Fluid density at the quadpoints or nodes
  const MaterialProperty<Real> & _density;

  /// d(Fluid density at the quadpoints or nodes)/d(porepressure at the quadpoints or nodes)
  const MaterialProperty<Real> & _ddensity_dp;

  /// d(Fluid density at the quadpoints or nodes)/d(temperature at the quadpoints or nodes)
  const MaterialProperty<Real> & _ddensity_dt;

  /// Fluid phase enthalpy at the qps or nodes
  MaterialProperty<Real> & _enthalpy;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> & _denthalpy_dp;

  /// Derivative of fluid enthalpy wrt temperature at the qps or nodes
  MaterialProperty<Real> & _denthalpy_dt;
};

#endif // POROUSFLOWENTHALPY_H
