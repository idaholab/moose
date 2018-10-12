//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTION_THERMAL_SOLID_PROPERTIES_H
#define FUNCTION_THERMAL_SOLID_PROPERTIES_H

#include "ThermalSolidProperties.h"
#include "Function.h"

class ThermalFunctionSolidProperties;

template <>
InputParameters validParams<ThermalFunctionSolidProperties>();

/**
 * Thermal material properties as a function of temperature from function
 * inputs. These functions are parameterized as a function of time such that
 * 't' corresponds to temperature, not time. This user object can also be
 * used to specify constant properties.
 */
class ThermalFunctionSolidProperties : public ThermalSolidProperties
{
public:
  ThermalFunctionSolidProperties(const InputParameters & parameters);

  /**
   * Solid name
   * @return "thermal_function"
   */
  virtual const std::string & solidName() const override;

  /**
   * Isobaric specific heat capacity as a function of temperature.
   * @param T temperature
   * @return isobaric specific heat
   */
  virtual Real cp_from_T(Real T) const override;

  /**
   * Isobaric specific heat capacity as a function of temperature and its
   * derivative with respect to temperature.
   * @param T temperature
   * @return cp isobaric specific heat
   * @return dcp_dT isobaric specific heat derivative with respect to temperature
   */
  virtual void cp_from_T(Real T, Real & cp, Real & dcp_dT) const override;

  /**
   * Thermal conductivity as a function of temperature.
   * @param T temperature
   * @return thermal conductivity
   */
  virtual Real k_from_T(Real T) const override;

  /**
   * Thermal conductivity as a function of temperature and its
   * derivative with respect to temperature.
   * @param T temperature
   * @return k thermal conductivity
   * @return dk_dT thermal conductivity derivative with respect to temperature
   */
  virtual void k_from_T(Real T, Real & k, Real & dk_dT) const override;

  /**
   * Density as a function of temperature.
   * @param T temperature
   * @return density
   */
  virtual Real rho_from_T(Real T) const override;

  /**
   * Density as a function of temperature and its
   * derivative with respect to temperature.
   * @param T temperature
   * @return rho density
   * @return drho_dT density derivative with respect to temperature
   */
  virtual void rho_from_T(Real T, Real & rho, Real & drho_dT) const override;

protected:
  /// Function providing the thermal conductivity as a function of temperature
  Function & _k;

  /// Function providing the isobaric specific heat as a function of temperature
  Function & _cp;

  /// Function providing the density as a function of temperature
  Function & _rho;

private:
  /// The solid name
  static const std::string _name;
};

#endif /* FUNCTION_THERMAL_SOLID_PROPERTIES_H */
