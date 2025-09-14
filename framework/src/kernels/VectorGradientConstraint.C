#include "VectorGradientConstraint.h"

registerMooseObject("MooseApp", VectorGradientConstraint);

InputParameters
VectorGradientConstraint::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "Enforces q = grad(u) where q is a vector field and u is a scalar field");
  params.addRequiredCoupledVar("u", "The scalar variable whose gradient we're constraining to");
  return params;
}

VectorGradientConstraint::VectorGradientConstraint(const InputParameters & parameters)
  : VectorKernel(parameters),
    _grad_coupled(coupledGradient("u")),
    _coupled_var_num(coupled("u"))
{
}

Real
VectorGradientConstraint::computeQpResidual()
{
  // Weak form: (q - grad(u), test) = 0
  // For vector kernels, _u[_qp] gives the vector value
  // and we need to take dot product with test vector
  return (_u[_qp] - _grad_coupled[_qp]) * _test[_i][_qp];
}

Real
VectorGradientConstraint::computeQpJacobian()
{
  // dR/dq = test * phi (both are vectors, so dot product)
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
VectorGradientConstraint::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _coupled_var_num)
  {
    // dR/du = -test · grad(phi)
    // For vector kernel test function dotted with gradient tensor
    Real result = 0.0;
    for (unsigned int d = 0; d < _mesh.dimension(); ++d)
      result -= _test[_i][_qp](d) * _grad_phi[_j][_qp](d, d);
    return result;
  }
  
  return 0.0;
}