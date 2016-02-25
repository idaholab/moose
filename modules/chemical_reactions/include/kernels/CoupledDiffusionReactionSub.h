/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Kernel.h"

#ifndef COUPLEDDIFFUSIONREACTIONSUB_H
#define COUPLEDDIFFUSIONREACTIONSUB_H

//Forward Declarations
class CoupledDiffusionReactionSub;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<CoupledDiffusionReactionSub>();

/**
 * Define the Kernel for a CoupledBEEquilibriumSub operator that looks like:
 *
 * grad (diff * grad (weight * 10^log_k * u^sto_u * v^sto_v)).
 */
class CoupledDiffusionReactionSub : public Kernel
{
public:

  /**
   * This is the Constructor declaration AND definition.
   * It is ok to have the definition in the .h if the function body
   * is really small.  Otherwise it should be in the .C
   */
  CoupledDiffusionReactionSub(const InputParameters & parameters);

protected:
  /**
   * Responsible for computing the residual at one quadrature point
   * This should always be defined in the .C
   * @return The residual of dispersion-diffusion of the coupled equilibrium species.
   */
  virtual Real computeQpResidual();

  /**
   * Responsible for computing the diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable this kernel operates on ("u").
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   *
   * This should always be defined in the .C
   * @return The diagonal jacobian of dispersion-diffusion of the coupled equilibrium species.
   */
  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /**
   * Coupled things come through as std::vector _refernces_.
   *
   * Since this is a reference it MUST be set in the Initialization List of the
   * constructor!
   */
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
