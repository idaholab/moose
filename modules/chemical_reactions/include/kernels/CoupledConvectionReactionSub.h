/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Kernel.h"

#ifndef COUPLEDCONVECTIONREACTIONSUB_H
#define COUPLEDCONVECTIONREACTIONSUB_H

// Forward Declarations
class CoupledConvectionReactionSub;

template <>
InputParameters validParams<CoupledConvectionReactionSub>();

/**
 * Define the Kernel for a CoupledConvectionReactionSub operator that looks like:
 * weight * cond * grad_pressure * 10^log_k * u^sto_u * v^sto_v
 */
class CoupledConvectionReactionSub : public Kernel
{
public:
  CoupledConvectionReactionSub(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// Weight of the equilibrium species concentration in the total primary species concentration.
  const Real _weight;

  /// Equilibrium constant for the equilibrium species in association form.
  const Real _log_k;

  /// Stoichiometric coefficient of the primary species.
  const Real _sto_u;

  /// Stoichiometric coefficients of the coupled primary species.
  const std::vector<Real> _sto_v;

  /// Material property of hydraulic conductivity.
  const MaterialProperty<Real> & _cond;

  /// Coupled gradient of hydraulic head.
  const VariableGradient & _grad_p;

  /// Coupled primary species variable numbers.
  std::vector<unsigned int> _vars;

  /// Coupled primary species concentrations.
  std::vector<const VariableValue *> _vals;

  /// Coupled gradients of primary species concentrations.
  std::vector<const VariableGradient *> _grad_vals;
};

#endif // COUPLEDCONVECTIONREACTIONSUB_H
