#include "ConvectionPrecompute.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<ConvectionPrecompute>()
{
  InputParameters params = validParams<KernelValue>();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  return params;
}

ConvectionPrecompute::ConvectionPrecompute(const std::string & name,
                       InputParameters parameters) :
    KernelValue(name, parameters),
    _velocity(getParam<RealVectorValue>("velocity"))
{}

Real
ConvectionPrecompute::precomputeQpResidual()
{
  // velocity * _grad_u[_qp] is actually doing a dot product
  return _velocity*_grad_u[_qp];
}

Real
ConvectionPrecompute::precomputeQpJacobian()
{
  // the partial derivative of _grad_u is just _grad_phi[_j]
  return _velocity*_grad_phi[_j][_qp];
}
