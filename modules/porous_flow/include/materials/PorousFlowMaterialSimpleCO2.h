/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALSIMPLECO2_H
#define POROUSFLOWMATERIALSIMPLECO2_H

#include "PorousFlowMaterialFluidPropertiesBase.h"

class PorousFlowMaterialSimpleCO2;

template<>
InputParameters validParams<PorousFlowMaterialSimpleCO2>();

/**
 * Simplified fluid properties of carbon dioxide (CO2).
 * Provides density, viscosity, derivatives wrt pressure and temperature,
 * and Henry Law constants.
 *
 * Note: These aren't as accurate as the full CO2 implementation, but are
 * fast and have simple analytical derivatives
 */
class PorousFlowMaterialSimpleCO2 : public PorousFlowMaterialFluidPropertiesBase
{
public:
  PorousFlowMaterialSimpleCO2(const InputParameters & parameters);

protected:

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /**
   * CO2 density as a function of pressure and temperature.
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return density (kg/m^3)
   */
  Real density(Real pressure, Real temperature) const;

  /**
   * CO2 viscosity as a function of  pressure and temperature.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @param density phase density (kg/m^3)
   * @return viscosity (Pa.s)
   */
  Real viscosity(Real pressure, Real temperature, Real density) const;

  /**
   * Derivative of the viscosity of CO2 as a function of temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @param density phase density (kg/m^3)
   * @return derivative of CO2 viscosity wrt temperature
   */
  Real dViscosity_dT(Real pressure, Real temperature, Real density) const;

  /**
   * Partial density of dissolved CO2. From Garcia, Density of aqueous
   * solutions of CO2, LBNL-49023 (2001).
   *
   * @param temperature fluid temperature (C)
   * @return partial molar density (kg/m^3)
   */
  Real partialDensity(Real temperature) const;

  /**
   * Density of supercritical CO2. From Ouyang, New correlations for predicting
   * the density and viscosity of supercritical Carbon Dioxide under conditions
   * expected in Carbon Capture and Sequestration operations, The Open Petroleum
   * Engineering Journal, 4, 13-21 (2011)
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return density (kg/m^3)
   */
  Real supercriticalDensity(Real pressure, Real temperature) const;

  /**
   * CO2 gas density as a function of  pressure and temperature.
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return density (kg/m^3)
   */
  Real gasDensity(Real pressure, Real temperature) const;

  /**
   * CO2 gas viscosity as a function of  pressure and temperature.
   * From Fenghour et al., The viscosity of carbon dioxide, J. Phys. Chem. Ref.
   * Data, 27, 31-44 (1998).
   * Note: used up to the critical point. The supercritical viscosity is calculated
   * in supercriticalViscosity(). As a result, the critical enhancement is not
   * implemented.
   *
   * @param temperature fluid temperature (C)
   * @param density fluid density (kg/m^3)
   * @return viscosity (Pa.s)
   */
  Real gasViscosity(Real temperature, Real density) const;

  /**
   * Viscosity of supercritical CO2. From Ouyang, New correlations for predicting
   * the density and viscosity of supercritical Carbon Dioxide under conditions
   * expected in Carbon Capture and Sequestration operations, The Open Petroleum
   * Engineering Journal, 4, 13-21 (2011)
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return viscosity (Pa.s)
   */
  Real supercriticalViscosity(Real pressure, Real temperature) const;

  /**
   * Derivative of the density of CO2 as a function of
   * pressure.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 density (kg/m^3) with respect to pressure (Pa)
   */
  Real dDensity_dP(Real pressure, Real temperature) const;

  /**
   * Derivative of the density of CO2 as a function of
   * temperature.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 density (kg/m^3) with respect to temperature (C)
   */
  Real dDensity_dT(Real pressure, Real temperature) const;

  /**
   * Derivative of the density of gaseous CO2 as a function of
   * pressure.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 density (kg/m^3) with respect to pressure (Pa)
   */
  Real dGasDensity_dP(Real pressure, Real temperature) const;

  /**
   * Derivative of the density of gaseous CO2 as a function of
   * temperature.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 density (kg/m^3) with respect to temperature (C)
   */
  Real dGasDensity_dT(Real pressure, Real temperature) const;

  /**
   * Derivative of the density of supercritical CO2 as a function of
   * pressure.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 density (kg/m^3) with respect to pressure (Pa)
   */
  Real dSupercriticalDensity_dP(Real pressure, Real temperature) const;

  /**
   * Derivative of the density of supercritical CO2 as a function of
   * temperature.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 density (kg/m^3) with respect to temperature (C)
   */
  Real dSupercriticalDensity_dT(Real pressure, Real temperature) const;

  /**
   * The derivative of CO2 viscosity with respect to density
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 viscosity wrt density
   */
  Real dViscosity_dDensity(Real pressure, Real temperature, Real density) const;

  /**
   * Derivative of the viscosity of gaseous CO2 as a function of density
   *
   * @param temperature fluid temperature (C)
   * @param density fluid density (kg/m^3)
   * @return derivative of CO2 viscosity wrt density
   */
  Real dGasViscosity_dDensity(Real temperature, Real density) const;

  /**
   * Derivative of the viscosity of supercritical CO2 as a function of density
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 viscosity wrt density
   */
  Real dSupercriticalViscosity_dDensity(Real pressure, Real temperature) const;

  /**
   * Derivative of the viscosity of supercritical CO2 as a function of pressure
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (C)
   * @return derivative of CO2 viscosity wrt pressure
   */
  Real dSupercriticalViscosity_dP(Real pressure, Real temperature) const;

  /**
   * Henry's law constant coefficients for dissolution of CO2 into water.
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
  /// CO2 molar mass (kg/mol)
  Real _Mco2;
  /// Critical pressure (Pa)
  Real _p_critical;
  /// Critical temperature (C)
  Real _t_critical;
};

#endif //POROUSFLOWMATERIALSIMPLECO2_H
