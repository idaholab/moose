//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWWATERNCG_H
#define POROUSFLOWWATERNCG_H

#include "PorousFlowFluidStateBase.h"

class Water97FluidProperties;
class SinglePhaseFluidPropertiesPT;
class PorousFlowWaterNCG;

template <>
InputParameters validParams<PorousFlowWaterNCG>();

/**
 * Specialized class for water and a non-condensable gas
 * Includes dissolution of gas in liquid water phase using Henry's law
 */
class PorousFlowWaterNCG : public PorousFlowFluidStateBase
{
public:
  PorousFlowWaterNCG(const InputParameters & parameters);

  /**
   * Name of FluidState
   * @return water-ncg
   */
  virtual std::string fluidStateName() const;

  void thermophysicalProperties(Real pressure,
                                Real temperature,
                                Real z,
                                std::vector<FluidStateProperties> & fsp) const;
  /**
   * Mass fractions of NCG in liquid phase and H2O in gas phase at thermodynamic
   * equilibrium. Calculated using Henry's law (for NCG component), and Raoult's
   * law (for water).
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (C)
   * @param[out] xncgl mass fraction of NCG in liquid (kg/kg)
   * @param[out] dxncgl_dp derivative of mass fraction of NCG in liquid wrt pressure
   * @param[out] dxncgl_dT derivative of mass fraction of NCG in liqiud wrt temperature
   * @param[out] xh2og mass fraction of H2O in gas (kg/kg)
   * @param[out] dh2ogl_dp derivative of mass fraction of NCG in gas wrt pressure
   * @param[out] dh2og_dT derivative of mass fraction of NCG in gas wrt temperature
   */
  void equilibriumMassFractions(Real pressure,
                                Real temperature,
                                Real & xncgl,
                                Real & dxncgl_dp,
                                Real & dxncgl_dT,
                                Real & xh2og,
                                Real & dxh2og_dp,
                                Real & dxh2og_dT) const;

  /**
   * Mass fractions of NCG and H2O in both phases, as well as derivatives wrt
   * PorousFlow variables. Values depend on the phase state (liquid, gas or two phase)
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (C)
   * @param z total mass fraction of NCG component
   * @param[out] PhaseStateEnum current phase state
   * @param[out] FluidStateMassFractions data structure
   */
  void massFractions(Real pressure,
                     Real temperature,
                     Real z,
                     FluidStatePhaseEnum & phase_state,
                     std::vector<FluidStateProperties> & fsp) const;

  /**
   * Gas density
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (C)
   * @param[out] FluidStateDensity data structure
   */
  void
  gasProperties(Real pressure, Real temperature, std::vector<FluidStateProperties> & fsp) const;

  /**
   * Liquid density
   *
   * @param pressure liquid pressure (Pa)
   * @param temperature temperature (C)
   * @param[out] FluidStateDensity data structure
   */
  void
  liquidProperties(Real pressure, Real temperature, std::vector<FluidStateProperties> & fsp) const;

  /**
   * Gas and liquid saturation in the two-phase region
   *
   * @param pressure gas pressure (Pa)
   * @param temperature phase temperature (C)
   * @param z total mass fraction of NCG component
   * @param[out] FluidStateSaturation data structure
   */
  void saturationTwoPhase(Real pressure,
                          Real temperature,
                          Real z,
                          std::vector<FluidStateProperties> & fsp) const;

  /**
   * Enthalpy of dissolution of NCG in water calculated using Henry's constant
   * From Himmelblau, Partial molal heats and entropies of solution for gases dissolved
   * in water from the freezing to the near critical point, J. Phys. Chem. 63 (1959)
   *
   * @param temperature fluid temperature (K)
   * @param Kh Henry's constant (Pa)
   * @param dKh_dT derivative of Henry's constant wrt temperature
   * @return enthalpy of dissolution (kJ/kg)
   */
  Real enthalpyOfDissolution(Real temperature, Real Kh, Real dKh_dT) const;

  /**
   * Total mass fraction of NCG summed over all phases in the two-phase state
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param saturation gas saturation (-)
   * @return total mass fraction z (-)
   */
  Real totalMassFraction(Real pressure, Real temperature, Real saturation) const;

protected:
  /**
   * Convert mole fraction to mass fraction
   * @param xmol mole fraction
   * @return mass fraction
   */
  Real moleFractionToMassFraction(Real xmol) const;

  /**
   * Check that the temperature is between the triple and critical values
   * @param temperature fluid temperature (K)
   */
  void checkVariables(Real temperature) const;

  /// Fluid properties UserObject for water
  const Water97FluidProperties & _water_fp;
  /// Fluid properties UserObject for the NCG
  const SinglePhaseFluidPropertiesPT & _ncg_fp;
  /// Molar mass of water (kg/mol)
  const Real _Mh2o;
  /// Molar mass of non-condensable gas (kg/mol)
  const Real _Mncg;
  /// Triple point temperature of water (K)
  const Real _water_triple_temperature;
  /// Critical temperature of water (K)
  const Real _water_critical_temperature;
};

#endif // POROUSFLOWWATERNCG_H
