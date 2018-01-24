/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DoubleWellPotential.h"

template <>
InputParameters
validParams<DoubleWellPotential>()
{
  InputParameters params = validParams<KernelValue>();
  params.addClassDescription(
      "Simple demonstration Allen-Cahn Kernel using an algebraic double-well potential");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");

  return params;
}

DoubleWellPotential::DoubleWellPotential(const InputParameters & parameters)
  : ACBulk<Real>(parameters)
{
}

Real
DoubleWellPotential::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _u[_qp] * _u[_qp] * _u[_qp] - _u[_qp];

    case Jacobian:
      return _phi[_j][_qp] * (3.0 * _u[_qp] * _u[_qp] - 1.0);
  }

  mooseError("Invalid type passed in");
}
