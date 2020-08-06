//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Diffusion of primary species in given equilibrium species
 */
class CoupledDiffusionReactionSub : public Kernel
{
public:
  static InputParameters validParams();

  CoupledDiffusionReactionSub(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// Material property of dispersion-diffusion coefficient
  const MaterialProperty<Real> & _diffusivity;
  /// Weight of the equilibrium species concentration in the total primary species concentration
  const Real _weight;
  /// Equilibrium constant for the equilibrium species in association form
  const VariableValue & _log_k;
  /// Stoichiometric coefficient of the primary species
  const Real _sto_u;
  /// Stoichiometric coefficients of the coupled primary species
  const std::vector<Real> _sto_v;
  /// Coupled primary species variable numbers
  const std::vector<unsigned int> _vars;
  /// Coupled primary species concentrations
  const std::vector<const VariableValue *> _vals;
  /// Coupled gradients of primary species concentrations
  const std::vector<const VariableGradient *> _grad_vals;
  /// Activity coefficient of primary species in the equilibrium species
  const VariableValue & _gamma_u;
  /// Activity coefficients of coupled primary species in the equilibrium species
  const std::vector<const VariableValue *> _gamma_v;
  /// Activity coefficient of equilibrium species
  const VariableValue & _gamma_eq;
};
