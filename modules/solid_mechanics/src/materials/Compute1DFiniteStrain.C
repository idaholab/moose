//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Compute1DFiniteStrain.h"

#include "libmesh/quadrature.h"

InputParameters
Compute1DFiniteStrain::validParams()
{
  InputParameters params = ComputeFiniteStrain::validParams();
  params.addClassDescription("Compute strain increment for finite strain in 1D problem");

  return params;
}

Compute1DFiniteStrain::Compute1DFiniteStrain(const InputParameters & parameters)
  : ComputeFiniteStrain(parameters)
{
}

void
Compute1DFiniteStrain::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Deformation gradient
    auto A = RankTwoTensor::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

    // Old Deformation gradient
    auto Fbar = RankTwoTensor::initializeFromRows(
        (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

    // Compute the displacement gradient dUy/dy and dUz/dz value for 1D problems
    A(1, 1) = computeGradDispYY();
    A(2, 2) = computeGradDispZZ();

    Fbar(1, 1) = computeGradDispYYOld();
    Fbar(2, 2) = computeGradDispZZOld();

    // Gauss point deformation gradient
    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);

    A -= Fbar; // very nearly A = gradU - gradUold, adapted to cylindrical coords

    Fbar.addIa(1.0); // Fbar = ( I + gradUold)

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);
  }

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpStrain();
}
