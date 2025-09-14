#pragma once

#include "TimeKernel.h"

namespace moose
{
namespace automatic_weak_form
{

/**
 * Time derivative kernel for variational problems
 * Computes time derivative contributions from energy functionals
 */
class VariationalTimeDerivative : public TimeKernel
{
public:
  static InputParameters validParams();

  VariationalTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  /// Coefficient for the time derivative (e.g., from energy functional)
  const Real _coeff;
  
  /// Whether to use automatic differentiation for Jacobian
  const bool _use_ad;
};

} // namespace automatic_weak_form
} // namespace moose