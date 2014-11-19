#include "WrongJacobianDiffusion.h"

template<>
InputParameters validParams<WrongJacobianDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("rfactor", 1.0, "Prefactor on the Residual");
  params.addParam<Real>("jfactor", 1.0, "Prefactor on the Jacobian");
  params.addCoupledVar("coupled", "optionally couple variables");
  return params;
}

WrongJacobianDiffusion::WrongJacobianDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _rfactor(getParam<Real>("rfactor")),
    _jfactor(getParam<Real>("jfactor"))
{
}

Real
WrongJacobianDiffusion::computeQpResidual()
{
  return _rfactor * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
WrongJacobianDiffusion::computeQpJacobian()
{
  return _jfactor * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
