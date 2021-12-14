//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Compute1DIncrementalStrain.h"

#include "libmesh/quadrature.h"

InputParameters
Compute1DIncrementalStrain::validParams()
{
  InputParameters params = ComputeIncrementalSmallStrain::validParams();
  params.addClassDescription("Compute strain increment for small strains in 1D problems.");

  return params;
}

Compute1DIncrementalStrain::Compute1DIncrementalStrain(const InputParameters & parameters)
  : ComputeIncrementalSmallStrain(parameters)
{
}

void
Compute1DIncrementalStrain::computeTotalStrainIncrement(RankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation for 1D problems
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

  total_strain_increment = 0.5 * (A + A.transpose());
}
