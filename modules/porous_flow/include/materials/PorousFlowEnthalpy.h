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

template<>
InputParameters validParams<PorousFlowEnthalpy>();

/**
 * This material computes specific enthalpy (J/kg) for a fluid assuming
 * using the relationship
 * specify_enthalpy = specific_internal_energy + pressure/density
 */
class PorousFlowEnthalpy : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowEnthalpy(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();

  virtual void computeQpProperties();

  /// Fluid energy at the nodes
  const MaterialProperty<Real> & _energy_nodal;

  /// d(Fluid energy at the nodes)/d(porepressure at the nodes)
  const MaterialProperty<Real> & _denergy_nodal_dp;

  /// d(Fluid energy at the nodes)/d(temperature at the nodes)
  const MaterialProperty<Real> & _denergy_nodal_dt;

  /// Fluid energy at the quadpoints
  const MaterialProperty<Real> & _energy_qp;

  /// d(Fluid energy at the quadpoints)/d(porepressure at the quadpoints)
  const MaterialProperty<Real> & _denergy_qp_dp;

  /// d(Fluid energy at the quadpoints)/d(temperature at the quadpoints)
  const MaterialProperty<Real> & _denergy_qp_dt;

  /// Fluid density at the nodes
  const MaterialProperty<Real> & _density_nodal;

  /// d(Fluid density at the nodes)/d(porepressure at the nodes)
  const MaterialProperty<Real> & _ddensity_nodal_dp;

  /// d(Fluid density at the nodes)/d(temperature at the nodes)
  const MaterialProperty<Real> & _ddensity_nodal_dt;

  /// Fluid density at the quadpoints
  const MaterialProperty<Real> & _density_qp;

  /// d(Fluid density at the quadpoints)/d(porepressure at the quadpoints)
  const MaterialProperty<Real> & _ddensity_qp_dp;

  /// d(Fluid density at the quadpoints)/d(temperature at the quadpoints)
  const MaterialProperty<Real> & _ddensity_qp_dt;

  /// Fluid phase enthalpy at the nodes
  MaterialProperty<Real> & _enthalpy_nodal;

  /// Old fluid phase enthalpy at the nodes
  MaterialProperty<Real> & _enthalpy_nodal_old;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _denthalpy_nodal_dp;

  /// Derivative of fluid enthalpy wrt temperature at the nodes
  MaterialProperty<Real> & _denthalpy_nodal_dt;

  /// Fluid phase enthalpy at the qps
  MaterialProperty<Real> & _enthalpy_qp;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the qps
  MaterialProperty<Real> & _denthalpy_qp_dp;

  /// Derivative of fluid enthalpy wrt temperature at the qps
  MaterialProperty<Real> & _denthalpy_qp_dt;
};

#endif // POROUSFLOWENTHALPY_H
