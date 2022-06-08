//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalSolidPropertiesMaterial.h"

/**
 * Monolithic silicon carbide properties as a function of temperature \cite snead.
 */
class ThermalSiliconCarbideProperties : public ThermalSolidPropertiesMaterial
{
public:
  static InputParameters validParams();

  ThermalSiliconCarbideProperties(const InputParameters & parameters);

  /**
   * Molar mass
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const override;

  /**
   * Isobaric specific heat capacity (J/kg$\cdot$K) as a function of temperature (K).
   * Correlation from \cite snead based on data in the range
   * 200 K $\le$ T $\le$ 2400 K. The uncertainty is $\pm$ 7%
   * in the range 200 K $\le$ T $\le$ 1000 K and $\pm$ 4% in the range
   * 1000 K $\le$ T $\le$ 2400 K. A wide variation in specific heat has been used
   * for modeling of the silicon carbide layer in TRISO fuel; this correlation
   * agrees reasonably well with the constant value of 1300 J/kg$\dot$K used
   * by Wang, but is about double the value of 620 J/kg$\cdot$K used by
   * Hales et. al \cite hales.
   */
  virtual void computeIsobaricSpecificHeat() override;

  /// Isobaric specific heat capacity derivatives
  virtual void computeIsobaricSpecificHeatDerivatives() override;

  /**
   * Thermal conductivity (W/m$\cdot$K) as a function of temperature (K).
   * The thermal conductivity of silicon carbide depends strongly on
   * the grain size, the nature of the grain boundaries,
   * and the presence of additives such as those included in sintering. For
   * monolithic silicon carbide, it is recommended to use single crystal or
   * chemical vapor deposition (CVD) properties (though even within CVD grades
   * there can be significant variation) \cite snead. For this reason,
   * manufacturer-specific information should be preferred to the correlations
   * available in this user object.
   *
   * Two correlations are available for use here - the correlation in \cite snead
   * pertaining to CVD silicon carbide, and a second correlation giving values
   * much closer to those typically used for modeling the silicon carbide in
   * TRISO fuels \cite parfume. At 650 $\degree$ C, the Snead correlation predicts
   * a thermal conductivity of about 106 W/m$\cdot$K, while the PARFUME correlation
   * predicts a value much closer to the constant values of 30 W/m$\cdot$K and
   * 16 W/m$\cdot$K used elsewhere \cite xin_wang_thesis \cite stainsby.
   *
   * A thermal diffusivity of $2.3e-4 \pm 1.1e-4$ m$^2$/s is measured by Rochais et. al
   * at room temperature \cite rochais. Assuming a density of 3216.0 kg/m$^3$ and a
   * specific heat of 676.7 J/kg$\cdot$K given by the other correlations in this
   * user object, Rochais et. al predict a thermal conductivity in the range of
   * 261 to 740 W/m$\cdot$K. At room temperature, Lopez-Honorato et. al predict
   * a thermal conductivity of 168 W/m$\cdot$K \cite lopez_honorato.
   * The Snead correlation prediction is closest to these values
   * at room temperature, while the PARFUME correlation is much lower. However, others
   * have measured 16.74, 50, and 62 W/m$\cdot$K \cite lopez_honorato, values
   * that are much closer to the PARFUME correlation predictions.
   */
  virtual void computeThermalConductivity() override;

  /// Thermal conductivity derivatives
  virtual void computeThermalConductivityDerivatives() override;

  /**
   * Density (kg/m$^3$) as a function of temperature (K).
   * The density is assumed constant due to the very small thermal expansion
   * coefficient of silicon carbide. A default density value is obtained by
   * averaging the density for four different silicon carbide crystal structures
   * at room temperature. This constant density agrees well with the constant
   * densities of 3.2 g/cm$^2$ \cite xin_wang_thesis and 3.18 g/cm$^3$
   * \cite tecdoc1694 \cite sun used by others.
   * @return rho density (kg/m$^3$)
   */
  virtual void computeDensity() override;

  /// Density derivatives
  virtual void computeDensityDerivatives() override;

protected:
  /// enumeration for selecting the thermal conductivity model
  enum ThermalSiliconCarbidePropertiesKModel
  {
    SNEAD,
    PARFUME
  };

  /// type of thermal conductivity model
  const ThermalSiliconCarbidePropertiesKModel _k_model;

  /// (constant) density
  const Real & _rho_const;

private:
  /// The solid name
  static const std::string _name;
};
