//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHMath.h"

registerMooseObject("PhaseFieldApp", CHMath);

InputParameters
CHMath::validParams()
{
  InputParameters params = CHBulk<Real>::validParams();
  params.addClassDescription(
      "Simple demonstration Cahn-Hilliard Kernel using an algebraic double-well potential");
  return params;
}

CHMath::CHMath(const InputParameters & parameters) : CHBulk<Real>(parameters) {}

RealGradient // Use This an example of the the function should look like
CHMath::computeGradDFDCons(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return 3 * _u[_qp] * _u[_qp] * _grad_u[_qp] - _grad_u[_qp]; // return Residual value

    case Jacobian:
      return 6 * _u[_qp] * _phi[_j][_qp] * _grad_u[_qp] +
             3 * _u[_qp] * _u[_qp] * _grad_phi[_j][_qp] -
             _grad_phi[_j][_qp]; // return Jacobian value
  }

  mooseError("Invalid type passed in");
}
