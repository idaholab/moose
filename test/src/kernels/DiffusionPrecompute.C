#include "DiffusionPrecompute.h"


template<>
InputParameters validParams<DiffusionPrecompute>()
{
  InputParameters p = validParams<KernelGrad>();
  return p;
}


DiffusionPrecompute::DiffusionPrecompute(const std::string & name, InputParameters parameters) :
    KernelGrad(name, parameters)
{
}

DiffusionPrecompute::~DiffusionPrecompute()
{
}

RealGradient
DiffusionPrecompute::precomputeQpResidual()
{
  // Note we do not multiple by the gradient of the test function.  That is done in the parent class
  return _grad_u[_qp];
}


RealGradient
DiffusionPrecompute::precomputeQpJacobian()
{
  // Note we do not multiple by the gradient of the test function.  That is done in the parent class
  return _grad_phi[_j][_qp];
}
