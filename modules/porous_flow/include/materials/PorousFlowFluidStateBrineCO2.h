/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDSTATEBRINECO2_H
#define POROUSFLOWFLUIDSTATEBRINECO2_H

#include "PorousFlowFluidStateFlashBase.h"

class BrineFluidProperties;
class SinglePhaseFluidPropertiesPT;
class PorousFlowFluidStateBrineCO2;

template <>
InputParameters validParams<PorousFlowFluidStateBrineCO2>();

/**
 * Fluid state class for brine and CO2. Includes mutual solubility of CO2 and
 * brine using model of Spycher, Pruess and Ennis-King, CO2-H2O mixtures in the
 * geological sequestration of CO2. I. Assessment and calculation of mutual
 * solubilities from 12 to 100C and up to 600 bar, Geochimica et Cosmochimica Acta,
 * 67, 3015-3031 (2003), and
 * Spycher and Pruess, CO2-H2O mixtures in the geological sequestration of CO2. II.
 * Partitioning in chloride brine at 12-100C and up to 600 bar, Geochimica et
 * Cosmochimica Acta, 69, 3309-3320 (2005)
 */
class PorousFlowFluidStateBrineCO2 : public PorousFlowFluidStateFlashBase
{
public:
  PorousFlowFluidStateBrineCO2(const InputParameters & parameters);

protected:
  virtual void thermophysicalProperties() override;
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
  void massFractions(Real pressure,
                     Real temperature,
                     Real xnacl,
                     Real & xco2l,
                     Real & dxco2l_dp,
                     Real & dxco2l_dT,
                     Real & xh2og,
                     Real & dxh2og_dp,
                     Real & dxh2og_dT) const;

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

  /// Salt mass fraction (kg/kg)
  const VariableValue & _xnacl;
  /// Fluid properties UserObject for brine
  const BrineFluidProperties & _brine_fp;
  /// Fluid properties UserObject for CO2
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

#endif // POROUSFLOWFLUIDSTATEBRINECO2_H
