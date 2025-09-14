#include "VariationalTimeDerivative.h"

namespace moose
{
namespace automatic_weak_form
{

registerMooseObject("MooseApp", VariationalTimeDerivative);

InputParameters
VariationalTimeDerivative::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addClassDescription("Time derivative kernel for variational problems - computes ∂u/∂t contributions");
  
  params.addParam<Real>("coefficient", 1.0, "Coefficient for the time derivative term");
  params.addParam<bool>("use_automatic_differentiation", false, "Use automatic differentiation for Jacobian computation");
  
  return params;
}

VariationalTimeDerivative::VariationalTimeDerivative(const InputParameters & parameters)
  : TimeKernel(parameters),
    _coeff(getParam<Real>("coefficient")),
    _use_ad(getParam<bool>("use_automatic_differentiation"))
{
}

Real
VariationalTimeDerivative::computeQpResidual()
{
  // For variational problems, the time derivative term is typically
  // coefficient * ∂u/∂t * test_function
  return _coeff * _u_dot[_qp] * _test[_i][_qp];
}

Real
VariationalTimeDerivative::computeQpJacobian()
{
  // The Jacobian for time derivative is 
  // coefficient * ∂(∂u/∂t)/∂u_j * test_function
  // where ∂(∂u/∂t)/∂u_j = du_dot_du * φ_j
  return _coeff * _phi[_j][_qp] * _du_dot_du[_qp] * _test[_i][_qp];
}

} // namespace automatic_weak_form
} // namespace moose