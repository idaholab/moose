//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STAINLESS_STEEL_316_PROPERTIES_H
#define STAINLESS_STEEL_316_PROPERTIES_H

#include "ThermalSolidProperties.h"
#include "SolidPropertiesEnums.h"

class ThermalStainlessSteel316Properties;

template <>
InputParameters validParams<ThermalStainlessSteel316Properties>();

/**
 * Stainless steel alloy 306 thermal solid properties as a function of
 * temperature \cite mills.
 *
 * The transition melting range is from 1385 C to 1450 C, so all data used here
 * is obtained form solid phase measurements and these properties should not
 * be used to compute transition or liquid regime properties.
 */
class ThermalStainlessSteel316Properties : public ThermalSolidProperties
{
public:
  ThermalStainlessSteel316Properties(const InputParameters & parameters);

  /**
   * Solid name
   * @return "thermal_stainless_steel_316"
   */
  virtual const std::string & solidName() const override;

  /**
   * Molar mass
   * The nominal chemical composition in weight percent is:
   * 0.08 C, 17.0 Cr, 0.3 Cu, 65.0 Fe, 2.0 Mn, 2.5 Mo, 12.0 Ni, 1.0 Si \cite mills.
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const override;

  /**
   * Isobaric specific heat capacity as a function of temperature.
   * A curve fit is performed from data in \cite mills over
   * 25 $\degree$C $\le$ T $\le$ 1300 $\degree$C. The fit is obtained with an
   * R$^2$ value of 0.9926. Uncertainty on the tabulated data is $\pm$ 5%.
   * @param T temperature (K)
   * @return isobaric specific heat (J/kg$\cdot$K)
   */
  virtual Real cp_from_T(Real T) const override;

  /**
   * Isobaric specific heat capacity as a function of temperature and its
   * derivative with respect to temperature.
   * @param T temperature (K)
   * @return cp isobaric specific heat (J/kg$\cdot$K)
   * @return dcp_dT isobaric specific heat derivative with respect to temperature (J/kg$\cdot$K$^2$)
   */
  virtual void cp_from_T(Real T, Real & cp, Real & dcp_dT) const override;

  /**
   * Thermal conductivity as a function of temperature.
   * A curve fit is performed from data in \cite mills over
   * 25 $\degree$ C $\le$ T $\le$ 1300 $\degree$ C. The fit is obtained with an
   * R$^2$ value of 0.9960. Uncertainty on the tabulated data is $\pm$ 10%.
   * @param T temperature (K)
   * @return thermal conductivity (W/m$\cdot$K)
   */
  virtual Real k_from_T(Real T) const override;

  /**
   * Thermal conductivity as a function of temperature and its
   * derivative with respect to temperature.
   * @param T temperature (K)
   * @return k thermal conductivity (W/m$\cdot$K)
   * @return dk_dT thermal conductivity derivative with respect to temperature (W/m$\cdot$K$^2$)
   */
  virtual void k_from_T(Real T, Real & k, Real & dk_dT) const override;

  /**
   * Density as a function of temperature.
   * A curve fit is performed from data in \cite mills over
   * 25 $\degree$ C $\le$ T $\le$ 1300 $\degree$ C. The fit is obtained with
   * an R$^2$ value of 0.9995. Uncertainty on the tabulated data is $\pm$ 3%.
   * @param T temperature (K)
   * @return density (kg/m$^3$)
   */
  virtual Real rho_from_T(Real T) const override;

  /**
   * Density as a function of temperature and its
   * derivative with respect to temperature.
   * @param T temperature (K)
   * @return rho density (kg/m$^3$)
   * @return drho_dT density derivative with respect to temperature (kg/m$^3$$\cdot$K)
   */
  virtual void rho_from_T(Real T, Real & rho, Real & drho_dT) const override;

  /**
   * Total integrated emissivity as a function of temperature
   * @param T temperature (K)
   * @return emissivity (unitless)
   */
  virtual Real emissivity_from_T(Real T) const override;

protected:
  /// The state of the surface, 'oxidized' or 'polished', used in estimating emissivity
  const surface::SurfaceEnum _surface;

  /// Whether the user has specified a constant emissivity
  const bool _constant_emissivity;

  /// Optional user-specified emissivity. If not set by the user, this is not used
  const Real & _emissivity;

private:
  /// The solid name
  static const std::string _name;
};

#endif /* STAINLESS_STEEL_316_PROPERTIES_H */
