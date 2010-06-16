#include "Diffusion.h"

template<>
InputParameters validParams<Diffusion>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


Diffusion::Diffusion(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
{}

Real
Diffusion::computeQpResidual()
{
  return _grad_test[_i][_qp]*_grad_u[_qp];
}

Real
Diffusion::computeQpJacobian()
{
  return _grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
