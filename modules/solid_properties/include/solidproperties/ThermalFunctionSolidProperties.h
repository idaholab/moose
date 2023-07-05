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
public:
  static InputParameters validParams();

  ThermalFunctionSolidProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  virtual Real k_from_T(const Real & T) const override;

  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;

  virtual Real cp_from_T(const Real & T) const override;

  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;

  virtual Real rho_from_T(const Real & T) const override;

  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;

protected:
  /// Function providing the thermal conductivity as a function of temperature
  const Function & _k_function;

  /// Function providing the isobaric specific heat as a function of temperature
  const Function & _cp_function;

  /// Function providing the density as a function of temperature
  const Function & _rho_function;
};

#pragma GCC diagnostic pop
