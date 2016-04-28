/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALCAPILLARYPRESSUREVGP_H
#define POROUSFLOWMATERIALCAPILLARYPRESSUREVGP_H

#include "PorousFlowMaterialCapillaryPressureBase.h"

//Forward Declarations
class PorousFlowMaterialCapillaryPressureVGP;

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureVGP>();

/**
 * van Genuchten capillary pressure for multiphase flow in porous media.
 * Note: this formulation is in terms of pore pressure, not saturation
 *
 * Based on van Genuchten, M. Th., A closed for equation for
 * predicting the hydraulic conductivity of unsaturated soils,
 * Soil Sci. Soc., 44, 892-898 (1980).
 */
class PorousFlowMaterialCapillaryPressureVGP : public PorousFlowMaterialCapillaryPressureBase
{
public:
  PorousFlowMaterialCapillaryPressureVGP(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  /// Effective phase saturation as a function of phase pore pressure
  Real effectiveSaturation(Real pressure) const;
  /// Derivative of effective saturation wrt phase pore pressure
  Real dEffectiveSaturation(Real pressure) const;
  /// Second derivative of effective saturation wrt phase pore pressure
  Real d2EffectiveSaturation(Real pressure) const;

  /// Name of (dummy) pressure primary variable
  VariableName _pressure_variable_name;
  /// Pressure material property at the nodes
  const MaterialProperty<std::vector<Real> > & _porepressure_nodal;
  /// Pressure material property at the qps
  const MaterialProperty<std::vector<Real> > & _porepressure_qp;
  /// Derivative of capillary pressure at the nodes wrt phase pore pressure
  MaterialProperty<Real> & _dcapillary_pressure_nodal_dp;
  /// Second derivative of capillary pressure at the nodes wrt pore pressure
  MaterialProperty<Real> & _d2capillary_pressure_nodal_dp2;

  /// van Genuchten alpha parameter
  Real _al;
  /// van Genuchten exponent m
  Real _m;
};

#endif //POROUSFLOWMATERIALCAPILLARYPRESSUREVGP_H
