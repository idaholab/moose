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

/**
 * Composite silicon carbide properties as a function of temperature.
 */
class ThermalCompositeSiCProperties : public ThermalSolidProperties
{
public:
  static InputParameters validParams();

  ThermalCompositeSiCProperties(const InputParameters & parameters);

  virtual Real k_from_T(const Real & T) const override;

  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;
  virtual void k_from_T(const DualReal & T, DualReal & k, DualReal & dk_dT) const override;

  virtual Real cp_from_T(const Real & T) const override;

  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;
  virtual void cp_from_T(const DualReal & T, DualReal & cp, DualReal & dcp_dT) const override;

  virtual Real rho_from_T(const Real & T) const override;

  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;
  virtual void rho_from_T(const DualReal & T, DualReal & rho, DualReal & drho_dT) const override;

protected:
  /// (constant) density
  const Real & _rho_const;
};
