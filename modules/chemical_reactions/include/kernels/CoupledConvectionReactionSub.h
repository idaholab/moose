/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Kernel.h"

#ifndef COUPLEDCONVECTIONREACTIONSUB_H
#define COUPLEDCONVECTIONREACTIONSUB_H

//Forward Declarations
class CoupledConvectionReactionSub;

template<>
InputParameters validParams<CoupledConvectionReactionSub>();

/**
 * Define the Kernel for a CoupledConvectionReactionSub operator that looks like:
 *
 * weight * cond * grad_pressure * 10^log_k * u^sto_u * v^sto_v
 *
 * This first line is defining the name and inheriting from Kernel.
 */
class CoupledConvectionReactionSub : public Kernel
{
public:
  CoupledConvectionReactionSub(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// Weight of the equilibrium species concentration in the total primary species concentration.
  Real _weight;

  /// Equilibrium constant for the equilibrium species in association form.
  Real _log_k;

  /// Stochiometric coefficient of the primary species.
  Real _sto_u;

  /// Stochiometric coefficiets of the coupled primary species.
  std::vector<Real> _sto_v;

  /// Material property of hydraulic conductivity.
  const MaterialProperty<Real> & _cond;

  /// Coupled gradient of hydraulic head.
  const VariableGradient & _grad_p;

  std::vector<unsigned int> _vars;

  /// Coupled primary species concentrations.
  std::vector<const VariableValue *> _vals;

  /// Coupled gradients of primary species concentrations.
  std::vector<const VariableGradient *> _grad_vals;
};

#endif //COUPLEDCONVECTIONREACTIONSUB_H
