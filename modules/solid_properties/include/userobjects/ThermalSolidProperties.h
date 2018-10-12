//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THERMALSOLIDPROPERTIES_H
#define THERMALSOLIDPROPERTIES_H

#include "SolidProperties.h"

class ThermalSolidProperties;

template <>
InputParameters validParams<ThermalSolidProperties>();

/**
 * Common class for thermal solid properties
 */
class ThermalSolidProperties : public SolidProperties
{
public:
  ThermalSolidProperties(const InputParameters & parameters);

  /**
   * Solid name
   * @return string representing solid name
   */
  virtual const std::string & solidName() const;

  /**
   * Isobaric (constant-pressure) specific heat from temperature
   *
   * @param[in] T       temperature
   */
  virtual Real cp_from_T(Real T) const;

  /**
   * Isobaric (constant-pressure) specific heat and its derivatives from temperature
   *
   * @param[in] T       temperature
   * @param[out] cp     isobaric specific heat
   * @param[out] dcp_dT derivative of isobaric specific heat with respect to temperature
   */
  virtual void cp_from_T(Real T, Real & cp, Real & dcp_dT) const;

  /**
   * Thermal conductivity from temperature
   *
   * @param[in] T       temperature
   */
  virtual Real k_from_T(Real T) const;

  /**
   * Thermal conductivity and its derivatives from temperature
   *
   * @param[in] T       temperature
   * @param[out] k      thermal conductivity
   * @param[out] dk_dT  derivative of thermal conductivity with respect to temperature
   */
  virtual void k_from_T(Real T, Real & k, Real & dk_dT) const;

  /**
   * Density from temperature
   *
   * @param[in] T       temperature
   */
  virtual Real rho_from_T(Real T) const;

  /**
   * Density and its derivatives from temperature
   *
   * @param[in] T       temperature
   * @param[out] rho    density
   * @param[out] drho_dT derivative of density with respect to temperature
   */
  virtual void rho_from_T(Real T, Real & rho, Real & drho_dT) const;

  /**
   * Thermal expansion coefficient,
   * $-\frac{1}{\rho}\frac{\partial\rho}{\partial T}$. By default this is calculated
   * simply according to its definition, but for some derived classes specific
   * correlations may be provided independently of density, or the thermal
   * expansion coefficient utilized in the calculation of density itself.
   * @param[in] T       temperature
   */
  virtual Real beta_from_T(Real T) const;

  /**
   * Emissivity from temperature
   *
   * @param[in] T       temperature
   */
  virtual Real emissivity_from_T(Real T) const;

private:
  /// The solid name
  static const std::string _name;
};

#endif /* THERMALSOLIDPROPERTIES_H */
