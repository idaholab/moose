//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivative.h"

/**
 * Time derivative of primary species in given equilibrium species
 */
class CoupledBEEquilibriumSub : public TimeDerivative
{
public:
  static InputParameters validParams();

  CoupledBEEquilibriumSub(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// Weight of the equilibrium species in the total primary species
  const Real _weight;
  /// Equilibrium constant for the equilibrium species
  const VariableValue & _log_k;
  /// Stoichiometric coefficient of the primary species in the equilibrium species
  const Real _sto_u;
  /// Stoichiometric coefficients of the coupled primary species in the equilibrium species
  const std::vector<Real> _sto_v;
  /// Activity coefficient of primary species in the equilibrium species
  const VariableValue & _gamma_u;
  /// Old activity coefficient of primary species in the equilibrium species
  const VariableValue & _gamma_u_old;
  /// Activity coefficients of coupled primary species in the equilibrium species
  const std::vector<const VariableValue *> _gamma_v;
  /// Old activity coefficients of coupled primary species in the equilibrium species
  const std::vector<const VariableValue *> _gamma_v_old;
  /// Activity coefficient of equilibrium species
  const VariableValue & _gamma_eq;
  /// Old activity coefficient of equilibrium species
  const VariableValue & _gamma_eq_old;
  /// Porosity
  const MaterialProperty<Real> & _porosity;
  /// Coupled primary species variable numbers
  const std::vector<unsigned int> _vars;
  /// Coupled primary species concentrations
  const std::vector<const VariableValue *> _v_vals;
  /// Old values of coupled primary species concentrations
  const std::vector<const VariableValue *> _v_vals_old;
  /// Old value of the primary species concentration.
  const VariableValue & _u_old;
};
