/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWBRINE_H
#define POROUSFLOWBRINE_H

#include "PorousFlowFluidPropertiesBase.h"
#include "BrineFluidProperties.h"

class PorousFlowBrine;

template <>
InputParameters validParams<PorousFlowBrine>();

/**
 * Fluid properties of Brine.
 * Provides density, viscosity, derivatives wrt pressure and temperature at the quadpoints or nodes
 */
class PorousFlowBrine : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowBrine(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Fluid phase density at the qps or nodes
  MaterialProperty<Real> & _density;

  /// Derivative of fluid density wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> & _ddensity_dp;

  /// Derivative of fluid density wrt temperature at the qps or nodes
  MaterialProperty<Real> & _ddensity_dT;

  /// Fluid phase viscosity at the nodes or qps
  MaterialProperty<Real> & _viscosity;

  /// Derivative of fluid phase viscosity wrt pressure at the nodes or qps
  MaterialProperty<Real> & _dviscosity_dp;

  /// Derivative of fluid phase viscosity wrt temperature at the nodes or qps
  MaterialProperty<Real> & _dviscosity_dT;

  /// Fluid phase internal_energy at the qps or nodes
  MaterialProperty<Real> & _internal_energy;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> & _dinternal_energy_dp;

  /// Derivative of fluid internal_energy wrt temperature at the qps or nodes
  MaterialProperty<Real> & _dinternal_energy_dT;

  /// Fluid phase enthalpy at the qps or nodes
  MaterialProperty<Real> & _enthalpy;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> & _denthalpy_dp;

  /// Derivative of fluid enthalpy wrt temperature at the qps or nodes
  MaterialProperty<Real> & _denthalpy_dT;

  /// Brine Fluid properties UserObject
  const BrineFluidProperties * _brine_fp;

  /// Water Fluid properties UserObject
  const SinglePhaseFluidPropertiesPT * _water_fp;

  /// NaCl mass fraction at the qps or nodes
  const VariableValue & _xnacl;
};

#endif // POROUSFLOWBRINE_H
