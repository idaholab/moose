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

template<>
InputParameters validParams<PorousFlowSingleComponentFluid>();

/**
 * General single component fluid material. Provides density, viscosity,
 * internal energy, enthalpy and derivatives wrt pressure and temperature
 * for a fluid defined in the FluidProperties module
 */
class PorousFlowSingleComponentFluid : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowSingleComponentFluid(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// Fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal;

  /// Old fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal_old;

  /// Derivative of fluid density wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _ddensity_nodal_dp;

  /// Derivative of fluid density wrt temperature at the nodes
  MaterialProperty<Real> & _ddensity_nodal_dT;

  /// Fluid phase density at the qps
  MaterialProperty<Real> & _density_qp;

  /// Derivative of fluid density wrt phase pore pressure at the qps
  MaterialProperty<Real> & _ddensity_qp_dp;

  /// Derivative of fluid density wrt temperature at the qps
  MaterialProperty<Real> & _ddensity_qp_dT;

  /// Fluid phase viscosity at the nodes
  MaterialProperty<Real> & _viscosity_nodal;

  /// Derivative of fluid phase viscosity wrt pressure at the nodes
  MaterialProperty<Real> & _dviscosity_nodal_dp;

  /// Derivative of fluid phase viscosity wrt temperature at the nodes
  MaterialProperty<Real> & _dviscosity_nodal_dT;

  /// Fluid phase internal_energy at the nodes
  MaterialProperty<Real> & _internal_energy_nodal;

  /// Old fluid phase internal_energy at the nodes
  MaterialProperty<Real> & _internal_energy_nodal_old;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _dinternal_energy_nodal_dp;

  /// Derivative of fluid internal_energy wrt temperature at the nodes
  MaterialProperty<Real> & _dinternal_energy_nodal_dT;

  /// Fluid phase internal_energy at the qps
  MaterialProperty<Real> & _internal_energy_qp;

  /// Derivative of fluid internal_energy wrt phase pore pressure at the qps
  MaterialProperty<Real> & _dinternal_energy_qp_dp;

  /// Derivative of fluid internal_energy wrt temperature at the qps
  MaterialProperty<Real> & _dinternal_energy_qp_dT;

  /// Fluid phase enthalpy at the nodes
  MaterialProperty<Real> & _enthalpy_nodal;

  /// Old fluid phase enthalpy at the nodes
  MaterialProperty<Real> & _enthalpy_nodal_old;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _denthalpy_nodal_dp;

  /// Derivative of fluid enthalpy wrt temperature at the nodes
  MaterialProperty<Real> & _denthalpy_nodal_dT;

  /// Fluid phase enthalpy at the qps
  MaterialProperty<Real> & _enthalpy_qp;

  /// Derivative of fluid enthalpy wrt phase pore pressure at the qps
  MaterialProperty<Real> & _denthalpy_qp_dp;

  /// Derivative of fluid enthalpy wrt temperature at the qps
  MaterialProperty<Real> & _denthalpy_qp_dT;

  /// Fluid properties UserObject
  const SinglePhaseFluidPropertiesPT & _fp;
};

#endif //POROUSFLOWSINGLECOMPONENTFLUID_H
