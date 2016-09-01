/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDDIFFUSIONREACTIONSUB_H
#define COUPLEDDIFFUSIONREACTIONSUB_H

#include "Kernel.h"

//Forward Declarations
class CoupledDiffusionReactionSub;

template<>
InputParameters validParams<CoupledDiffusionReactionSub>();

/**
 * Define the Kernel for a CoupledBEEquilibriumSub operator that looks like:
 * grad (diff * grad (weight * 10^log_k * u^sto_u * v^sto_v)).
 */
class CoupledDiffusionReactionSub : public Kernel
{
public:
  CoupledDiffusionReactionSub(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// Material property of dispersion-diffusion coefficient.
  const MaterialProperty<Real> & _diffusivity;

  /// Weight of the equilibrium species concentration in the total primary species concentration.
  Real _weight;

  /// Equilibrium constant for the equilibrium species in association form.
  Real _log_k;

  /// Stochiometric coefficient of the primary species.
  Real _sto_u;

  /// Stochiometric coefficiets of the coupled primary species.
  std::vector<Real> _sto_v;

  std::vector<unsigned int> _vars;

  /// Coupled primary species concentrations.
  std::vector<const VariableValue *> _vals;

  /// Coupled gradients of primary species concentrations.
  std::vector<const VariableGradient *> _grad_vals;
};

#endif //COUPLEDDIFFUSIONREACTIONSUB_H
