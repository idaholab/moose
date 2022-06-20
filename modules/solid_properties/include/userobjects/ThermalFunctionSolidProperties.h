//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalSolidProperties.h"

class Function;

/**
 * Thermal material properties as a function of temperature from function
 * inputs. These functions are parameterized as a function of time such that
 * 't' corresponds to temperature, not time. This user object can also be
 * used to specify constant properties.
 */
class ThermalFunctionSolidProperties : public ThermalSolidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  static InputParameters validParams();

  ThermalFunctionSolidProperties(const InputParameters & parameters);

  /**
   * Thermal conductivity from temperature
   *
   * @param[in] T temperature
   * @return thermal conductivity
   */
  virtual Real k_from_T(const Real & T) const override;

  /**
   * Thermal conductivity and its derivatives from temperature
   *
   * @param[in] T   temperature
   * @param[out]    thermal conductivity
   * @param[out]    derivative of thermal conductivity w.r.t. temperature
   */
  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;
  virtual void k_from_T(const DualReal & T, DualReal & k, DualReal & dk_dT) const override;

  /**
   * Isobaric specific heat capacity from temperature
   *
   * @param[in] T temperature
   * @return isobaric specific heat capacity
   */
  virtual Real cp_from_T(const Real & T) const override;

  /**
   * Isobaric specific heat capacity and its derivatives from temperature
   *
   * @param[in] T   temperature
   * @param[out]    isobaric specific heat capacity
   * @param[out]    derivative of isobaric specific heat capacity w.r.t. temperature
   */
  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;
  virtual void cp_from_T(const DualReal & T, DualReal & cp, DualReal & dcp_dT) const override;

  /**
   * Density from temperature
   *
   * @param[in] T temperature
   * @return density
   */
  virtual Real rho_from_T(const Real & T) const override;

  /**
   * Density and its derivatives from temperature
   *
   * @param[in] T   temperature
   * @param[out]    density
   * @param[out]    derivative of density w.r.t. temperature
   */
  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;
  virtual void rho_from_T(const DualReal & T, DualReal & rho, DualReal & drho_dT) const override;

protected:
  /// Function providing the thermal conductivity as a function of temperature
  const Function & _k_function;

  /// Function providing the isobaric specific heat as a function of temperature
  const Function & _cp_function;

  /// Function providing the density as a function of temperature
  const Function & _rho_function;
};
