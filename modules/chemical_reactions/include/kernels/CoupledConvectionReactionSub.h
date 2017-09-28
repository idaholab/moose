/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COUPLEDCONVECTIONREACTIONSUB_H
#define COUPLEDCONVECTIONREACTIONSUB_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class CoupledConvectionReactionSub;

template <>
InputParameters validParams<CoupledConvectionReactionSub>();

/**
 * Define the Kernel for a CoupledConvectionReactionSub operator that looks like:
 * weight * velocity * 10^log_k * u^sto_u * v^sto_v
 */
class CoupledConvectionReactionSub : public DerivativeMaterialInterface<Kernel>
{
public:
  CoupledConvectionReactionSub(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Weight of the equilibrium species concentration in the total primary species concentration
  const Real _weight;

  /// Equilibrium constant for the equilibrium species in association form
  const Real _log_k;

  /// Stoichiometric coefficient of the primary species
  const Real _sto_u;

  /// Stoichiometric coefficients of the coupled primary species
  const std::vector<Real> _sto_v;

  /// Hydraulic conductivity
  const MaterialProperty<Real> & _cond;

  /// Gravity
  const RealVectorValue _gravity;

  /// Fluid density
  const MaterialProperty<Real> & _density;

  /// Pressure gradient
  const VariableGradient & _grad_p;

  /// Pressure variable number
  const unsigned int _pvar;

  /// Coupled primary species variable numbers
  std::vector<unsigned int> _vars;

  /// Coupled primary species concentrations
  std::vector<const VariableValue *> _vals;

  /// Coupled gradients of primary species concentrations
  std::vector<const VariableGradient *> _grad_vals;
};

#endif // COUPLEDCONVECTIONREACTIONSUB_H
