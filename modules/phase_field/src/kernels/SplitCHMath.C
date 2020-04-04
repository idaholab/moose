//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitCHMath.h"

registerMooseObject("PhaseFieldApp", SplitCHMath);

InputParameters
SplitCHMath::validParams()
{
  InputParameters params = SplitCHCRes::validParams();
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
