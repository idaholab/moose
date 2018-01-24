//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWBRINECO2_H
#define POROUSFLOWBRINECO2_H

#include "PorousFlowFluidStateBase.h"

class BrineFluidProperties;
class SinglePhaseFluidPropertiesPT;
class PorousFlowBrineCO2;

template <>
InputParameters validParams<PorousFlowBrineCO2>();

/**
 * Specialized class for brine and CO2 including calculation of mutual
 * solubility of the two fluids using the high-accuracy formulation of
 * Spycher, Pruess and Ennis-King, CO2-H2O mixtures in the geological
 * sequestration of CO2. I. Assessment and calculation of mutual solubilities
 * from 12 to 100C and up to 600 bar, Geochimica et Cosmochimica Acta, 67, 3015-3031 (2003),
 * and Spycher and Pruess, CO2-H2O mixtures in the geological sequestration of CO2. II.
 * Partitioning in chloride brine at 12-100C and up to 600 bar, Geochimica et
 * Cosmochimica Acta, 69, 3309-3320 (2005)
 */
class PorousFlowBrineCO2 : public PorousFlowFluidStateBase
{
public:
  PorousFlowBrineCO2(const InputParameters & parameters);

  /**
   * Name of FluidState
   * @return brine-co2
   */
  virtual std::string fluidStateName() const;

  void thermophysicalProperties(Real pressure,
                                Real temperature,
                                Real xnacl,
                                Real z,
                                std::vector<FluidStateProperties> & fsp) const;
  /**
   * Mass fractions of CO2 and brine calculated using mutual solubilities given
   * by Spycher, Pruess and Ennis-King, CO2-H2O mixtures in the geological
   * sequestration of CO2. I. Assessment and calculation of mutual solubilities
   * from 12 to 100C and up to 600 bar, Geochimica et Cosmochimica Acta, 67, 3015-3031 (2003),
   * and Spycher and Pruess, CO2-H2O mixtures in the geological sequestration of CO2. II.
   * Partitioning in chloride brine at 12-100C and up to 600 bar, Geochimica et
   * Cosmochimica Acta, 69, 3309-3320 (2005)
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (C)
   * @param xnacl NaCl mass fraction (kg/kg)
   * @param[out] xco2l mass fraction of CO2 in liquid (kg/kg)
   * @param[out] dxco2l_dp derivative of mass fraction of CO2 in liquid wrt pressure
   * @param[out] dxco2l_dT derivative of mass fraction of CO2 in liqiud wrt temperature
   * @param[out] xh2og mass fraction of H2O in gas (kg/kg)
   * @param[out] dh2ogl_dp derivative of mass fraction of H2O in gas wrt pressure
   * @param[out] dh2og_dT derivative of mass fraction of H2O in gas wrt temperature
   */
  void equilibriumMassFractions(Real pressure,
                                Real temperature,
                                Real xnacl,
                                Real & xncgl,
                                Real & dxncgl_dp,
                                Real & dxncgl_dT,
                                Real & xh2og,
                                Real & dxh2og_dp,
                                Real & dxh2og_dT) const;

  /**
   * Mass fractions of CO2 and H2O in both phases, as well as derivatives wrt
   * PorousFlow variables. Values depend on the phase state (liquid, gas or two phase)
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (C)
   * @param xnacl NaCl mass fraction (kg/kg)
   * @param z total mass fraction of CO2 component
   * @param[out] PhaseStateEnum current phase state
   * @param[out] FluidStateMassFractions data structure
   */
  void massFractions(Real pressure,
                     Real temperature,
                     Real xnacl,
                     Real z,
                     FluidStatePhaseEnum & phase_state,
                     std::vector<FluidStateProperties> & fsp) const;

  /**
   * Thermophysical properties of the gaseous state
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (C)
   * @param xnacl NaCl mass fraction (kg/kg)
   * @param[out] FluidStateDensity data structure
   */
  void
  gasProperties(Real pressure, Real temperature, std::vector<FluidStateProperties> & fsp) const;

  /**
   * Thermophysical properties of the liquid state
   *
   * @param pressure liquid pressure (Pa)
   * @param temperature temperature (C)
   * @param xnacl NaCl mass fraction (kg/kg)
   * @param[out] FluidStateDensity data structure
   */
  void liquidProperties(Real pressure,
                        Real temperature,
                        Real xnacl,
                        std::vector<FluidStateProperties> & fsp) const;

  /**
   * Gas and liquid saturations for the two-phase region
   *
   * @param pressure gas pressure (Pa)
   * @param temperature phase temperature (C)
   * @param xnacl NaCl mass fraction (kg/kg)
   * @param z total mass fraction of CO2 component
   * @param[out] FluidStateSaturation data structure
   */
  void saturationTwoPhase(Real pressure,
                          Real temperature,
                          Real xnacl,
                          Real z,
                          std::vector<FluidStateProperties> & fsp) const;

  /**
   * Fugacity coefficient for CO2. Eq. (B7) from Spycher, Pruess and
   * Ennis-King, CO2-H2O mixtures in the geological sequestration of CO2. I.
   * Assessment and calculation of mutual solubilities from 12 to 100C and
   * up to 600 bar, Geochimica et Cosmochimica Acta, 67, 3015-3031 (2003)
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param[out] fco2 fugacity coefficient for CO2
   * @param[out] dfco2_dp derivative of fugacity coefficient wrt pressure
   * @param[out] dfco2_dT derivative of fugacity coefficient wrt temperature
   */
  void fugacityCoefficientCO2(
      Real pressure, Real temperature, Real & fco2, Real & dfco2_dp, Real & dfco2_dT) const;

  /**
   * Fugacity coefficient for H2O. Eq. (B7) from Spycher, Pruess and
   * Ennis-King, CO2-H2O mixtures in the geological sequestration of CO2. I.
   * Assessment and calculation of mutual solubilities from 12 to 100C and
   * up to 600 bar, Geochimica et Cosmochimica Acta, 67, 3015-3031 (2003)
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param[out] fh2o fugacity coefficient for H2O
   * @param[out] dfh2o_dp derivative of fugacity coefficient wrt pressure
   * @param[out] dfh2o_dT derivative of fugacity coefficient wrt temperature
   */
  void fugacityCoefficientH2O(
      Real pressure, Real temperature, Real & fh2o, Real & dfh2o_dp, Real & dfh2o_dT) const;

  /**
   * Activity coefficient for CO2 in brine. From Duan and Sun, An improved model calculating
   * CO2 solubility in pure water and aqueous NaCl solutions from 257 to 533 K and from 0 to
   * 2000 bar, Chem. Geol., 193, 257-271 (2003)
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (K)
   * @param xnacl salt mass fraction (kg/kg)
   * @param[out] gamma activity coefficient for CO2 in brine (output)
   * @param[out] dgamma_dp derivative of activity coefficient wrt pressure
   * @param[out] dgamma_dT derivative of activity coefficient wrt temperature
   */
  void activityCoefficient(Real pressure,
                           Real temperature,
                           Real xnacl,
                           Real & gamma,
                           Real & dgamma_dp,
                           Real & dgamma_dT) const;

  /**
   * Equilibrium constant for H2O from Spycher, Pruess and
   * Ennis-King, CO2-H2O mixtures in the geological sequestration of CO2. I.
   * Assessment and calculation of mutual solubilities from 12 to 100C and
   * up to 600 bar, Geochimica et Cosmochimica Acta, 67, 3015-3031 (2003).
   * Note: correlation uses temperature in Celcius
   *
   * @param temperature temperature (C)
   * @param[out] kh2o equilibrium constant for H2O
   * @param[out] dkh2o_dT derivative of equilibrium constant wrt temperature
   */
  void equilibriumConstantH2O(Real temperature, Real & kh2o, Real & dkh2o_dT) const;

  /**
   * Equilibrium constant for CO2 from Spycher, Pruess and
   * Ennis-King, CO2-H2O mixtures in the geological sequestration of CO2. I.
   * Assessment and calculation of mutual solubilities from 12 to 100C and
   * up to 600 bar, Geochimica et Cosmochimica Acta, 67, 3015-3031 (2003).
   * Note: correlation uses temperature in Celcius
   *
   * @param temperature temperature (C)
   * @param[out] kco2 equilibrium constant for CO2
   * @param[out] dkco2_dT derivative of equilibrium constant wrt temperature
   */
  void equilibriumConstantCO2(Real temperature, Real & kco2, Real & dkco2_dT) const;

  /**
   * Partial density of dissolved CO2
   * From Garcia, Density of aqueous solutions of CO2, LBNL-49023 (2001)
   *
   * @param temperature fluid temperature (K)
   * @param[out] partial molar density (kg/m^3)
   * @param[out] derivative of partial molar density wrt temperature
   */
  void
  partialDensityCO2(Real temperature, Real & partial_density, Real & dpartial_density_dT) const;

  /**
   * Total mass fraction of CO2 summed over all phases in the two-phase state
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param xnacl NaCl mass fraction (kg/kg)
   * @param saturation gas saturation (-)
   * @return total mass fraction z (-)
   */
  Real totalMassFraction(Real pressure, Real temperature, Real xnacl, Real saturation) const;

protected:
  /// Check the input variables
  void checkVariables(Real pressure, Real temperature) const;

  /// Fluid properties UserObject for water
  const BrineFluidProperties & _brine_fp;
  /// Fluid properties UserObject for the CO2
  const SinglePhaseFluidPropertiesPT & _co2_fp;
  /// Fluid properties UserObject for H20
  const SinglePhaseFluidPropertiesPT & _water_fp;
  /// Molar mass of water (kg/mol)
  const Real _Mh2o;
  /// Inverse of molar mass of H2O (mol/kg)
  const Real _invMh2o;
  /// Molar mass of CO2 (kg/mol)
  const Real _Mco2;
  /// Molar mass of NaCL
  const Real _Mnacl;
  /// Molar gas constant in bar cm^3 /(K mol)
  const Real _Rbar;
};

#endif // POROUSFLOWBRINECO2_H
