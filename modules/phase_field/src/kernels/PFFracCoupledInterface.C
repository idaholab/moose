#include "PFFracCoupledInterface.h"

template<>
InputParameters validParams<PFFracCoupledInterface>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addRequiredCoupledVar("c", "Order parameter for damage");

  return params;
}

PFFracCoupledInterface::PFFracCoupledInterface(const std::string & name,
                                               InputParameters parameters):
  KernelGrad(name, parameters),
  _c(coupledValue("c")),
  _grad_c(coupledGradient("c")),
  _c_var(coupled("c"))
{
}

RealGradient
PFFracCoupledInterface::precomputeQpResidual()
{
  return  _grad_c[_qp];
}

RealGradient
PFFracCoupledInterface::precomputeQpJacobian()
{
  return 0.0;
}
/**
 * Contributes only to the off-diagonal Jacobian term
 * for auxiliary variable beta
 */
Real
PFFracCoupledInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return _grad_test[_i][_qp] * _grad_phi[_j][_qp];
  else
    return 0.0;
}
