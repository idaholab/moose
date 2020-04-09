//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHBulkPFCTrad.h"

registerMooseObject("PhaseFieldApp", CHBulkPFCTrad);

InputParameters
CHBulkPFCTrad::validParams()
{
  InputParameters params = CHBulk<Real>::validParams();
  params.addClassDescription(
      "Cahn-Hilliard kernel for a polynomial phase field crystal free energy.");
  return params;
}

CHBulkPFCTrad::CHBulkPFCTrad(const InputParameters & parameters)
  : CHBulk<Real>(parameters),
    _C0(getMaterialProperty<Real>("C0")),
    _a(getMaterialProperty<Real>("a")),
    _b(getMaterialProperty<Real>("b"))
{
}

RealGradient
CHBulkPFCTrad::computeGradDFDCons(PFFunctionType type)
{
  Real d2fdc2 = 1.0 - _C0[_qp] - _a[_qp] * _u[_qp] + _b[_qp] * _u[_qp] * _u[_qp];

  switch (type)
  {
    case Residual:
      return d2fdc2 * _grad_u[_qp];

    case Jacobian:
    {
      Real d3fdc3 = -_a[_qp] + 2.0 * _b[_qp] * _u[_qp];
      return d2fdc2 * _grad_phi[_j][_qp] + d3fdc3 * _grad_u[_qp] * _phi[_j][_qp];
    }
  }

  mooseError("Invalid type passed in");
}
