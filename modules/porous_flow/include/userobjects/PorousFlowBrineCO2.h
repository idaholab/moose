//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFluidStateMultiComponentBase.h"

class BrineFluidProperties;
class SinglePhaseFluidProperties;
class Water97FluidProperties;

/**
 * Specialized class for brine and CO2 including calculation of mutual
 * solubility of the two fluids using a high-accuracy fugacity-based formulation.
 *
 * For temperatures 12C <= T <= 99C, the formulation is based on
 * Spycher, Pruess and Ennis-King, CO2-H2O mixtures in the geological
 * sequestration of CO2. I. Assessment and calculation of mutual solubilities
 * from 12 to 100C and up to 600 bar, Geochimica et Cosmochimica Acta, 67, 3015-3031 (2003),
 * and Spycher and Pruess, CO2-H2O mixtures in the geological sequestration of CO2. II.
 * Partitioning in chloride brine at 12-100C and up to 600 bar, Geochimica et
 * Cosmochimica Acta, 69, 3309-3320 (2005).
 *
 * For temperatures 109C <= T <= 300C, the formulation is based on
 * Spycher and Pruess, A Phase-Partitioning Model for CO2-Brine Mixtures at Elevated
 * Temperatures and Pressures: Application to CO2-Enhanced Geothermal Systems,
 * Transport in Porous Media, 82, 173-196 (2010)
 *
 * As the two formulations do not coincide at temperatures near 100C, a cubic
 * polynomial is used in the intermediate temperature range 99C < T < 109C to
 * provide a smooth transition from the two formulations in this region.
 *
 * Notation convention
 * Throughout this class, both mole fractions and mass fractions will be used.
 * The following notation will be used:
 * yk: mole fraction of component k in the gas phase
 * xk: mole fraction of component k in the liquid phase
 * Yk: mass fraction of component k in the gas phase
 * Xk: mass fraction of component k in the liquid phase
 */
class PorousFlowBrineCO2 : public PorousFlowFluidStateMultiComponentBase
{
public:
  static InputParameters validParams();

  PorousFlowBrineCO2(const InputParameters & parameters);

  virtual std::string fluidStateName() const override;

  void thermophysicalProperties(Real pressure,
                                Real temperature,
                                Real Xnacl,
                                Real Z,
                                unsigned int qp,
                                std::vector<FluidStateProperties> & fsp) const override;

  void thermophysicalProperties(const ADReal & pressure,
                                const ADReal & temperature,
                                const ADReal & Xnacl,
                                const ADReal & Z,
                                unsigned int qp,
                                std::vector<FluidStateProperties> & fsp) const override;

  /**
   * Mole fractions of CO2 in brine and water vapor in CO2 at equilibrium
   *
   * In the low temperature regime (T <= 99C), the mole fractions are calculated directly,
   * while in the elevated temperature regime (T >= 109C), they are calculated iteratively.
   * In the intermediate regime (99C < T < 109C), a cubic polynomial is used to smoothly
   * connect the low and elevated temperature regimes.
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param[out] xco2 mole fraction of CO2 in liquid
   * @param[out] yh2o mole fraction of H2O in gas
   */
  void equilibriumMoleFractions(const ADReal & pressure,
                                const ADReal & temperature,
                                const ADReal & Xnacl,
                                ADReal & xco2,
                                ADReal & yh2o) const;

  /**
   * Mass fractions of CO2 in brine and water vapor in CO2 at equilibrium
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param[out] Xco2 mass fraction of CO2 in liquid (kg/kg)
   * @param[out] Yh2o mass fraction of H2O in gas (kg/kg)
   */
  void equilibriumMassFractions(const ADReal & pressure,
                                const ADReal & temperature,
                                const ADReal & Xnacl,
                                ADReal & Xco2,
                                ADReal & Yh2o) const;

  /**
   * Mass fractions of CO2 and H2O in both phases, as well as derivatives wrt
   * PorousFlow variables. Values depend on the phase state (liquid, gas or two phase)
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param Z total mass fraction of CO2 component
   * @param[out] PhaseStateEnum current phase state
   * @param[out] FluidStateProperties data structure
   */
  void massFractions(const ADReal & pressure,
                     const ADReal & temperature,
                     const ADReal & Xnacl,
                     const ADReal & Z,
                     FluidStatePhaseEnum & phase_state,
                     std::vector<FluidStateProperties> & fsp) const;

  /**
   * As above, but also returns the equilibrium composition computed on the way, so that it
   * does not have to be computed a second time for the absent phase (absentPhaseProperties())
   *
   * @param[out] Xco2_eq equilibrium mass fraction of CO2 in the liquid (kg/kg)
   * @param[out] Yh2o_eq equilibrium mass fraction of H2O in the gas (kg/kg)
   * @param[out] eq_computed false if Z is below the smallest solubility, in which case the
   * equilibrium composition is not computed and the two values above are not set
   */
  void massFractions(const ADReal & pressure,
                     const ADReal & temperature,
                     const ADReal & Xnacl,
                     const ADReal & Z,
                     FluidStatePhaseEnum & phase_state,
                     std::vector<FluidStateProperties> & fsp,
                     ADReal & Xco2_eq,
                     ADReal & Yh2o_eq,
                     bool & eq_computed) const;

  /**
   * Thermophysical properties of the gaseous state
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param[out] FluidStateProperties data structure
   */
  void gasProperties(const ADReal & pressure,
                     const ADReal & temperature,
                     std::vector<FluidStateProperties> & fsp) const;

  /**
   * Thermophysical properties of the liquid state
   *
   * @param pressure liquid pressure (Pa)
   * @param temperature temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param[out] FluidStateProperties data structure
   */
  void liquidProperties(const ADReal & pressure,
                        const ADReal & temperature,
                        const ADReal & Xnacl,
                        std::vector<FluidStateProperties> & fsp) const;

  /**
   * Properties of the phase that is absent in a single-phase state.
   *
   * An absent phase has zero saturation, so it carries no mass and no advective flux. Its
   * density and composition are nevertheless used in terms that are not weighted by the
   * saturation, such as the gravity head of a face in the finite-volume kernels and the phase
   * composition in the diffusive flux. Leaving them at zero makes those terms jump when the
   * phase appears or disappears, so the residual is discontinuous at the phase boundary and
   * Newton can cycle between the one- and two-phase states (see #34412). Here the absent phase
   * is given the equilibrium composition and the density, viscosity and (optionally) enthalpy
   * it would have at the current pressure and temperature, which makes these properties
   * continuous across the phase boundary. The saturation of the absent phase stays zero.
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param phase_state current phase state (LIQUID: the gas phase is absent, GAS: the liquid
   * phase is absent)
   * @param Xco2_eq equilibrium mass fraction of CO2 in the liquid, from massFractions()
   * @param Yh2o_eq equilibrium mass fraction of H2O in the gas, from massFractions()
   * @param eq_computed whether the two values above were computed by massFractions(); if not,
   * the equilibrium composition is computed here
   * @param[out] FluidStateProperties data structure
   */
  void absentPhaseProperties(const ADReal & pressure,
                             const ADReal & temperature,
                             const ADReal & Xnacl,
                             FluidStatePhaseEnum phase_state,
                             const ADReal & Xco2_eq,
                             const ADReal & Yh2o_eq,
                             bool eq_computed,
                             std::vector<FluidStateProperties> & fsp) const;

  /**
   * Gas saturation in the two-phase region
   *
   * @param pressure gas pressure (Pa)
   * @param temperature phase temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param Z total mass fraction of CO2 component
   * @param FluidStateProperties data structure
   * @return gas saturation (-)
   */
  ADReal saturation(const ADReal & pressure,
                    const ADReal & temperature,
                    const ADReal & Xnacl,
                    const ADReal & Z,
                    std::vector<FluidStateProperties> & fsp) const;

  /**
   * Gas and liquid properties in the two-phase region
   *
   * @param pressure gas pressure (Pa)
   * @param temperature phase temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param Z total mass fraction of NCG component
   * @param qp quadpoint for capillary presssure
   * @param[out] FluidStateProperties data structure
   */
  void twoPhaseProperties(const ADReal & pressure,
                          const ADReal & temperature,
                          const ADReal & Xnacl,
                          const ADReal & Z,
                          unsigned int qp,
                          std::vector<FluidStateProperties> & fsp) const;

  /**
   * Fugacity coefficients for H2O and CO2 for T <= 100C
   * Eq. (B7) from Spycher, Pruess and Ennis-King (2003)
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param co2_density CO2 density (kg/m^3)
   * @param[out] fco2 fugacity coefficient for CO2
   * @param[out] fh2o fugacity coefficient for H2O
   */
  void fugacityCoefficientsLowTemp(const ADReal & pressure,
                                   const ADReal & temperature,
                                   const ADReal & co2_density,
                                   ADReal & fco2,
                                   ADReal & fh2o) const;

  ///@{
  /**
   * Fugacity coefficients for H2O and CO2 at elevated temperatures (100C < T <= 300C).
   * Eq. (A8) from Spycher & Pruess (2010)
   *
   * @param pressure gas pressure (Pa)
   * @param temperature temperature (K)
   * @param co2_density CO2 density (kg/m^3)
   * @param xco2 mole fraction of CO2 in liquid phase (-)
   * @param yh2o mole fraction of H2O in gas phase (-)
   * @param[out] fco2 fugacity coefficient for CO2
   * @param[out] fh2o fugacity coefficient for H2O
   */
  void fugacityCoefficientsHighTemp(const ADReal & pressure,
                                    const ADReal & temperature,
                                    const ADReal & co2_density,
                                    const ADReal & xco2,
                                    const ADReal & yh2o,
                                    ADReal & fco2,
                                    ADReal & fh2o) const;

  ADReal fugacityCoefficientH2OHighTemp(const ADReal & pressure,
                                        const ADReal & temperature,
                                        const ADReal & co2_density,
                                        const ADReal & xco2,
                                        const ADReal & yh2o) const;

  ADReal fugacityCoefficientCO2HighTemp(const ADReal & pressure,
                                        const ADReal & temperature,
                                        const ADReal & co2_density,
                                        const ADReal & xco2,
                                        const ADReal & yh2o) const;
  ///@}

  /**
   * Activity coefficient of H2O
   * Eq. (12) from Spycher & Pruess (2010)
   *
   * @param temperature temperature (K)
   * @param xco2 mole fraction of CO2 in liquid phase (-)
   * @return activity coefficient
   */
  ADReal activityCoefficientH2O(const ADReal & temperature, const ADReal & xco2) const;

  /**
   * Activity coefficient of CO2
   * Eq. (13) from Spycher & Pruess (2010)
   *
   * @param temperature temperature (K)
   * @param xco2 mole fraction of CO2 in liquid phase (-)
   * @return activity coefficient
   */
  ADReal activityCoefficientCO2(const ADReal & temperature, const ADReal & xco2) const;

  /**
   * Activity coefficient for CO2 in brine. From Duan and Sun, An improved model calculating
   * CO2 solubility in pure water and aqueous NaCl solutions from 257 to 533 K and from 0 to
   * 2000 bar, Chem. Geol., 193, 257-271 (2003)
   *
   * Note: this is not a 'true' activity coefficient, and is instead related to the molality
   * of CO2 in water and brine. Nevertheless, Spycher and Pruess (2005) refer to it as an
   * activity coefficient, so this notation is followed here.
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (K)
   * @param Xnacl salt mass fraction (kg/kg)
   * @return activity coefficient for CO2 in brine
   */
  ADReal activityCoefficient(const ADReal & pressure,
                             const ADReal & temperature,
                             const ADReal & Xnacl) const;

  /**
   * Activity coefficient for CO2 in brine used in the elevated temperature formulation.
   * Eq. (18) from Spycher and Pruess (2010).
   *
   * Note: unlike activityCoefficient(), no pressure dependence is included in
   * this formulation
   *
   * @param temperature phase temperature (K)
   * @param Xnacl salt mass fraction (kg/kg)
   * @return activity coefficient for CO2 in brine
   */
  ADReal activityCoefficientHighTemp(const ADReal & temperature, const ADReal & Xnacl) const;

  /**
   * Equilibrium constant for H2O
   * For temperatures 12C <= T <= 99C, uses Spycher, Pruess and Ennis-King (2003)
   * For temperatures 109C <= T <= 300C, uses Spycher and Pruess (2010)
   * For temperatures 99C < T < 109C, the value is calculated by smoothly interpolating
   * the two formulations
   *
   * @param temperature temperature (K)
   * @return equilibrium constant for H2O
   */
  ADReal equilibriumConstantH2O(const ADReal & temperature) const;

  /**
   * Equilibrium constant for CO2
   * For temperatures 12C <= T <= 99C, uses Spycher, Pruess and Ennis-King (2003)
   * For temperatures 109C <= T <= 300C, uses Spycher and Pruess (2010)
   * For temperatures 99C < T < 109C, the value is calculated by smoothly interpolating
   * the two formulations
   *
   * @param temperature temperature (K)
   * @return equilibrium constant for CO2
   */
  ADReal equilibriumConstantCO2(const ADReal & temperature) const;

  /**
   * Partial density of dissolved CO2
   * From Garcia, Density of aqueous solutions of CO2, LBNL-49023 (2001)
   *
   * @param temperature fluid temperature (K)
   * @return partial molar density (kg/m^3)
   */
  ADReal partialDensityCO2(const ADReal & temperature) const;

  virtual Real totalMassFraction(
      Real pressure, Real temperature, Real Xnacl, Real saturation, unsigned int qp) const override;

  /**
   * The index of the salt component
   * @return salt component number
   */
  unsigned int saltComponentIndex() const { return _salt_component; };

  /**
   * Henry's constant of dissolution of gas phase CO2 in brine. From
   * Battistelli et al, A fluid property module for the TOUGH2 simulator for saline brines
   * with non-condensible gas, Proc. Eighteenth Workshop on Geothermal Reservoir Engineering (1993)
   *
   * @param temperature fluid temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @return Henry's constant (Pa)
   */
  ADReal henryConstant(const ADReal & temperature, const ADReal & Xnacl) const;

  /**
   * Enthalpy of dissolution of gas phase CO2 in brine calculated using Henry's constant
   * From Himmelblau, Partial molal heats and entropies of solution for gases dissolved
   * in water from the freezing to the near critical point, J. Phys. Chem. 63 (1959).
   * Correction due to salinity from Battistelli et al, A fluid property module for the
   * TOUGH2 simulator for saline brines with non-condensible gas, Proc. Eighteenth Workshop
   * on Geothermal Reservoir Engineering (1993).
   *
   * @param temperature fluid temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @return enthalpy of dissolution (J/kg)
   */
  ADReal enthalpyOfDissolutionGas(const ADReal & temperature, const ADReal & Xnacl) const;

  /**
   * Enthalpy of dissolution of CO2 in brine calculated using linear fit to model of
   * Duan and Sun, An improved model calculating CO2 solubility in pure water and aqueous NaCl
   * solutions from 273 to 533 K and from 0 to 2000 bar, Chemical geology, 193, 257--271 (2003).
   *
   * In the region of interest, the more complicated model given in Eq. (8) of Duan and Sun
   * is well approximated by a simple linear fit (R^2 = 0.9997).
   *
   * Note: as the effect of salt mass fraction is small, it is not included in this function.
   *
   * @param temperature fluid temperature (K)
   * @return enthalpy of dissolution (J/kg)
   */
  ADReal enthalpyOfDissolution(const ADReal & temperature) const;

  /**
   * Mole fractions of CO2 in brine and water vapor in CO2 at equilibrium in the low
   * temperature regime (T <= 99C).
   *
   * @param pressure phase pressure (Pa)
   * @param temperature phase temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param[out] xco2 mole fraction of CO2 in liquid
   * @param[out] yh2o mass fraction of mole in gas
   */
  void equilibriumMoleFractionsLowTemp(const ADReal & pressure,
                                       const ADReal & temperature,
                                       const ADReal & Xnacl,
                                       ADReal & xco2,
                                       ADReal & yh2o) const;

  /**
   * Function to solve for yh2o and xco2 iteratively in the elevated temperature regime (T > 100C)
   *
   * @param pressure gas pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param co2_density CO2 density (kg/m^3)
   * @param[out] xco2 mole fraction of CO2 in liquid phase (-)
   * @param[out] yh2o mole fraction of H2O in gas phase (-)
   */
  void solveEquilibriumMoleFractionHighTemp(Real pressure,
                                            Real temperature,
                                            Real Xnacl,
                                            Real co2_density,
                                            Real & xco2,
                                            Real & yh2o) const;

protected:
  /// Check the input variables
  virtual void checkVariables(Real pressure, Real temperature) const;

  /**
   * Cubic function to smoothly interpolate between the low temperature and elevated
   * temperature models for 99C < T < 109C
   *
   * @param temperature temperature (K)
   * @param f0 function value at T = 372K (99C)
   * @param df0 derivative of function at T = 372K (99C)
   * @param f1 function value at T = 382K (109C)
   * @param df1 derivative of function at T = 382K (109C)
   * @param[out] value value at the given temperature
   * @param[out] deriv derivative at the given temperature
   */
  void smoothCubicInterpolation(
      Real temperature, Real f0, Real df0, Real f1, Real df1, Real & value, Real & deriv) const;

  ///@{
  /**
   * The function A (Eq. (11) of Spycher, Pruess and Ennis-King (2003) for T <= 100C,
   * and Eqs. (10) and (17) of Spycher and Pruess (2010) for T > 100C)
   *
   * @param pressure gas pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param Xnacl NaCl mass fraction (kg/kg)
   * @param co2_density CO2 density (kg/m^3)
   * @param xco2 mole fraction of CO2 in liquid phase (-)
   * @param yh2o mole fraction of H2O in gas phase (-)
   * @param[out] A the function A
   * @param[out] B the function B
   */
  void funcABHighTemp(Real pressure,
                      Real temperature,
                      Real Xnacl,
                      Real co2_density,
                      Real xco2,
                      Real yh2o,
                      Real & A,
                      Real & B) const;

  void funcABHighTemp(Real pressure,
                      Real temperature,
                      Real Xnacl,
                      Real co2_density,
                      Real xco2,
                      Real yh2o,
                      Real & A,
                      Real & dA_dp,
                      Real & dA_dT,
                      Real & B,
                      Real & dB_dp,
                      Real & dB_dT,
                      Real & dB_dX) const;

  void funcABLowTemp(const ADReal & pressure,
                     const ADReal & temperature,
                     const ADReal & co2_density,
                     ADReal & A,
                     ADReal & B) const;
  ///@}

  /// Salt component index
  const unsigned int _salt_component;
  /// Fluid properties UserObject for water
  const BrineFluidProperties & _brine_fp;
  /// Fluid properties UserObject for the CO2
  const SinglePhaseFluidProperties & _co2_fp;
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
  /// Temperature below which the Spycher, Pruess & Ennis-King (2003) model is used (K)
  const Real _Tlower;
  /// Temperature above which the Spycher & Pruess (2010) model is used (K)
  const Real _Tupper;
  /// Minimum Z - below this value all CO2 will be dissolved. This reduces the
  /// computational burden when small values of Z are present
  const Real _Zmin;
  /// Henry's coefficeients for CO2
  const std::vector<Real> _co2_henry;
};
