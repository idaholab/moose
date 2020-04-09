//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitCHBase.h"

InputParameters
SplitCHBase::validParams()
{
  InputParameters params = Kernel::validParams();

  return params;
}

SplitCHBase::SplitCHBase(const InputParameters & parameters) : Kernel(parameters) {}

/*Real //Example of what the virtual function should look like
SplitCHBase::computeDFDC(PFFunctionType type)
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
SplitCHBase::computeQpResidual()
{
  Real f_prime_zero = computeDFDC(Residual);
  Real e_prime = computeDEDC(Residual);

  Real residual = (f_prime_zero + e_prime) * _test[_i][_qp];

  return residual;
}

Real
SplitCHBase::computeQpJacobian()
{
  Real df_prime_zero_dc = computeDFDC(Jacobian);
  Real de_prime_dc = computeDEDC(Jacobian);

  Real jacobian = (df_prime_zero_dc + de_prime_dc) * _test[_i][_qp];

  return jacobian;
}

Real
SplitCHBase::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}

Real SplitCHBase::computeDFDC(PFFunctionType /*type*/) { return 0.0; }

Real SplitCHBase::computeDEDC(PFFunctionType /*type*/) { return 0.0; }
