/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDBEEQUILIBRIUMSUB_H
#define COUPLEDBEEQUILIBRIUMSUB_H

#include "Kernel.h"

// Forward Declarations
class CoupledBEEquilibriumSub;

template <>
InputParameters validParams<CoupledBEEquilibriumSub>();

/**
 * Define the Kernel for a CoupledBEEquilibriumSub operator that looks like:
 * delta (weight * 10^log_k * u^sto_u * v^sto_v) / delta t.
 */
class CoupledBEEquilibriumSub : public Kernel
{
public:
  CoupledBEEquilibriumSub(const InputParameters & parameters);

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

  /// Material property of porosity.
  const MaterialProperty<Real> & _porosity;

  /// Coupled primary species variable numbers.
  std::vector<unsigned int> _vars;

  /// Coupled primary species concentrations.
  std::vector<const VariableValue *> _v_vals;

  /// Coupled old values of primary species concentrations.
  std::vector<const VariableValue *> _v_vals_old;

  /// The old values of the primary species concentration.
  const VariableValue & _u_old;
};

#endif // COUPLEDBEEQUILIBRIUMSUB_H
