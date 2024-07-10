//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DoubleWellPotential.h"

registerMooseObject("PhaseFieldApp",DoubleWellPotential);

InputParameters
DoubleWellPotential::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription(
      "Simple demonstration Allen-Cahn Kernel using an algebraic double-well potential");
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
