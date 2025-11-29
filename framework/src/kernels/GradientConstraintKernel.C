#include "GradientConstraintKernel.h"

registerMooseObject("MooseApp", GradientConstraintKernel);

InputParameters
GradientConstraintKernel::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "Enforces the constraint q = ∇u for variable splitting in higher-order PDEs");
  params.addRequiredCoupledVar("coupled_variable", 
      "The scalar variable whose gradient this vector variable should equal");
  params.addParam<unsigned int>("component", 0, "Component index (0=x, 1=y, 2=z)");
  return params;
}

GradientConstraintKernel::GradientConstraintKernel(const InputParameters & parameters)
  : VectorKernel(parameters),
    _grad_coupled(coupledGradient("coupled_variable")),
    _coupled_var_num(coupled("coupled_variable")),
    _component(getParam<unsigned int>("component"))
{
}

Real
GradientConstraintKernel::computeQpResidual()
{
  // Residual: q - ∇u = 0
  // In weak form: (q - ∇u) · test = 0
  // For vector kernels, _u[_qp] is a vector and _test[_i][_qp] is a vector
  return (_u[_qp] - _grad_coupled[_qp]) * _test[_i][_qp];
}

Real
GradientConstraintKernel::computeQpJacobian()
{
  // Jacobian with respect to q
  // d/dq[(q - ∇u) · test] = test · φ
  return _test[_i][_qp] * _phi[_j][_qp];
}

Real
GradientConstraintKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Off-diagonal Jacobian with respect to u
  if (jvar == _coupled_var_num)
  {
    // d/du[(q - ∇u) · test] = -test · ∇φ
    // Need to compute test vector dotted with gradient of shape function
    Real result = 0.0;
    for (unsigned int d = 0; d < _mesh.dimension(); ++d)
      result -= _test[_i][_qp](d) * _grad_phi[_j][_qp](d, 0);
    return result;
  }
  
  return 0.0;
}