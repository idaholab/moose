/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALMETHANE_H
#define POROUSFLOWMATERIALMETHANE_H

#include "PorousFlowMaterialFluidPropertiesBase.h"

class PorousFlowMaterialMethane;

template<>
InputParameters validParams<PorousFlowMaterialMethane>();

/**
 * Fluid properties of Methane (CH4).
 * Provides density, viscosity, derivatives wrt pressure and temperature,
 * and Henry Law constants.
 */
class PorousFlowMaterialMethane : public PorousFlowMaterialFluidPropertiesBase
{
public:
  PorousFlowMaterialMethane(const InputParameters & parameters);

protected:

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /**
   * Methane gas density as a function of pressure and temperature
   * (assuming an ideal gas).
   *
   * @param pressure gas pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return density (kg/m^3)
   */
  Real density(Real pressure, Real temperature) const;

  /**
   * Derivative of the density of gaseous CH4 as a function of
   * pressure.
   *
   * @param temperature gas temperature (C)
   * @return derivative of CH4 density (kg/m^3) with respect to pressure
   */
  Real dDensity_dP(Real temperature) const;

  /**
   * Derivative of the density of gaseous CH4 as a function of
   * temperature.
   *
   * @param pressure gas pressure (Pa)
   * @param temperature gas temperature (C)
   * @return derivative of CH4 density (kg/m^3) with respect to temperature
   */
  Real dDensity_dT(Real pressure, Real temperature) const;

  /**
   * Methane gas viscosity as a function of temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param temperature fluid temperature (C)
   * @return viscosity (Pa.s)
   */
  Real viscosity(Real temperature) const;

  /**
   * Derivative of the viscosity of gaseous CH4 as a function of temperature
   *
   * @param temperature gas temperature (C)
   * @return derivative of CH4 viscosity wrt temperature
   */
  Real dViscosity_dT(Real temperature) const;

  /**
   * Henry's law constant coefficients for dissolution of CH4 into water.
   * From Guidelines on the Henry's constant and vapour
   * liquid distribution constant for gases in H20 and D20 at high
   * temperatures, IAPWS (2004).
   *
   * @return constants for Henry's constant (-)
   */
  std::vector<Real> henryConstants() const;

  /// Fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal;
  /// Old fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal_old;
  /// Derivative of fluid density wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _ddensity_nodal_dp;
  /// Derivative of fluid density wrt temperature at the nodes
  MaterialProperty<Real> & _ddensity_nodal_dt;
  /// Fluid phase density at the qps
  MaterialProperty<Real> & _density_qp;
  /// Derivative of fluid density wrt phase pore pressure at the qps
  MaterialProperty<Real> & _ddensity_qp_dp;
  /// Derivative of fluid density wrt temperature at the qps
  MaterialProperty<Real> & _ddensity_qp_dt;
  /// Fluid phase viscosity at the nodes
  MaterialProperty<Real> & _viscosity_nodal;
  /// Derivative of fluid phase viscosity wrt temperature at the nodes
  MaterialProperty<Real> & _dviscosity_nodal_dt;
  /// Methane molar mass (kg/mol)
  Real _Mch4;
};

#endif //POROUSFLOWMATERIALMETHANE_H
