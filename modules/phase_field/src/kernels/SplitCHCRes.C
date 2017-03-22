/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SplitCHCRes.h"

template <>
InputParameters
validParams<SplitCHCRes>()
{
  InputParameters params = validParams<SplitCHBase>();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel");
  params.addRequiredCoupledVar("w", "chem poten");
  params.addRequiredParam<MaterialPropertyName>("kappa_name", "The kappa used with the kernel");
  return params;
}

SplitCHCRes::SplitCHCRes(const InputParameters & parameters)
  : SplitCHBase(parameters),
    _kappa(getMaterialProperty<Real>("kappa_name")),
    _w_var(coupled("w")),
    _w(coupledValue("w"))
{
}

/*Real
SplitCHCRes::computeDFDC(PFFunctionType type)
{
  switch (type)
  {
  case Residual:
    return _u[_qp]*_u[_qp]*_u[_qp] - _u[_qp]; // return Residual value

  case Jacobian:
    return (3.0*_u[_qp]*_u[_qp] - 1.0)*_phi[_j][_qp]; //return Jacobian value

  }

  mooseError("Invalid type passed in");
}*/

Real
SplitCHCRes::computeQpResidual()
{
  Real residual =
      SplitCHBase::computeQpResidual(); //(f_prime_zero+e_prime)*_test[_i][_qp] from SplitCHBase

  residual += -_w[_qp] * _test[_i][_qp];
  residual += _kappa[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];

  return residual;
}

Real
SplitCHCRes::computeQpJacobian()
{
  Real jacobian = SplitCHBase::computeQpJacobian(); //(df_prime_zero_dc+de_prime_dc)*_test[_i][_qp];
                                                    // from SplitCHBase

  jacobian += _kappa[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];

  return jacobian;
}

Real
SplitCHCRes::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _w_var)
  {
    return -_phi[_j][_qp] * _test[_i][_qp];
  }

  return 0.0;
}
