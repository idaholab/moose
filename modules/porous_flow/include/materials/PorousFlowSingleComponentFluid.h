/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWSINGLECOMPONENTFLUID_H
#define POROUSFLOWSINGLECOMPONENTFLUID_H

#include "PorousFlowFluidPropertiesBase.h"
#include "SinglePhaseFluidPropertiesPT.h"

class PorousFlowSingleComponentFluid;

template <>
InputParameters validParams<PorousFlowSingleComponentFluid>();

/**
 * General single component fluid material. Provides quadpoint density, viscosity,
 * internal energy, enthalpy and derivatives wrt pressure and temperature
 * for a fluid defined in the FluidProperties module
 */
class PorousFlowSingleComponentFluid : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowSingleComponentFluid(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Fluid phase density at the qps or nodes
  MaterialProperty<Real> & _density;

  /// Derivative of fluid density wrt phase pore pressure at the qps or nodes
  MaterialProperty<Real> & _ddensity_dp;

  /// Derivative of fluid density wrt temperature at the qps or nodes
  MaterialProperty<Real> & _ddensity_dT;

  /// Fluid phase viscosity at the nodes
  MaterialProperty<Real> & _viscosity;

  /// Derivative of fluid phase viscosity wrt pressure at the nodes
  MaterialProperty<Real> & _dviscosity_dp;

  /// Derivative of fluid phase viscosity wrt temperature at the nodes
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

  /// Fluid properties UserObject
  const SinglePhaseFluidPropertiesPT & _fp;
};

#endif // POROUSFLOWSINGLECOMPONENTFLUID_H
