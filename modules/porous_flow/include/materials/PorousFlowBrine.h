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

template<>
InputParameters validParams<PorousFlowBrine>();

/**
 * Fluid properties of Brine.
 * Provides density, viscosity, derivatives wrt pressure and temperature at the quadpoints
 */
class PorousFlowBrine : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowBrine(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Fluid phase density at the qps
  MaterialProperty<Real> & _density_qp;

  /// Derivative of fluid density wrt phase pore pressure at the qps
  MaterialProperty<Real> & _ddensity_qp_dp;

  /// Derivative of fluid density wrt temperature at the qps
  MaterialProperty<Real> & _ddensity_qp_dT;

  /// Fluid phase internal_energy at the qps
  MaterialProperty<Real> & _internal_energy_qp;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the qps
  MaterialProperty<Real> & _dinternal_energy_qp_dp;

  /// Derivative of fluid internal_energy wrt temperature at the qps
  MaterialProperty<Real> & _dinternal_energy_qp_dT;

  /// Fluid phase enthalpy at the qps
  MaterialProperty<Real> & _enthalpy_qp;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the qps
  MaterialProperty<Real> & _denthalpy_qp_dp;

  /// Derivative of fluid enthalpy wrt temperature at the qps
  MaterialProperty<Real> & _denthalpy_qp_dT;

  /// Brine Fluid properties UserObject
  const BrineFluidProperties * _brine_fp;

  /// Water Fluid properties UserObject
  const SinglePhaseFluidPropertiesPT * _water_fp;

  /// NaCl mass fraction at the qps
  const VariableValue & _xnacl_qp;
};

#endif //POROUSFLOWBRINE_H
