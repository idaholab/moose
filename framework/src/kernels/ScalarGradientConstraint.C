#include "ScalarGradientConstraint.h"

registerMooseObject("MooseApp", ScalarGradientConstraint);

InputParameters
ScalarGradientConstraint::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Enforces q = ∂u/∂x_i for a specific component of the gradient");
  params.addRequiredCoupledVar("u", "The variable whose gradient component we're constraining to");
  params.addParam<unsigned int>("component", 0, "Gradient component (0=x, 1=y, 2=z)");
  return params;
}

ScalarGradientConstraint::ScalarGradientConstraint(const InputParameters & parameters)
  : Kernel(parameters),
    _grad_coupled(coupledGradient("u")),
    _coupled_var_num(coupled("u")),
    _component(getParam<unsigned int>("component"))
{
  if (_component >= _mesh.dimension())
    mooseError("Component ", _component, " is out of range for ", _mesh.dimension(), "D mesh");
}

Real
ScalarGradientConstraint::computeQpResidual()
{
  // Weak form: (q - ∂u/∂x_i, test) = 0
  return (_u[_qp] - _grad_coupled[_qp](_component)) * _test[_i][_qp];
}

Real
ScalarGradientConstraint::computeQpJacobian()
{
  // ∂R/∂q = test * φ
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
ScalarGradientConstraint::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _coupled_var_num)
  {
    // ∂R/∂u = -test * ∂φ/∂x_i
    return -_grad_phi[_j][_qp](_component) * _test[_i][_qp];
  }
  
  return 0.0;
}