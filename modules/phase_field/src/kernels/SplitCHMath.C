/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SplitCHMath.h"

template <>
InputParameters
validParams<SplitCHMath>()
{
  InputParameters params = validParams<SplitCHCRes>();
  params.addClassDescription("Simple demonstration split formulation Cahn-Hilliard Kernel using an "
                             "algebraic double-well potential");
  return params;
}

SplitCHMath::SplitCHMath(const InputParameters & parameters) : SplitCHCRes(parameters) {}

Real
SplitCHMath::computeDFDC(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _u[_qp] * _u[_qp] * _u[_qp] - _u[_qp]; // return Residual value

    case Jacobian:
      return (3.0 * _u[_qp] * _u[_qp] - 1.0) * _phi[_j][_qp]; // return Jacobian value
  }

  mooseError("Invalid type passed in");
}
