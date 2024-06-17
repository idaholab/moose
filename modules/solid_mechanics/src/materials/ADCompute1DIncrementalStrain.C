//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCompute1DIncrementalStrain.h"

#include "libmesh/quadrature.h"

InputParameters
ADCompute1DIncrementalStrain::validParams()
{
  InputParameters params = ADComputeIncrementalStrain::validParams();
  params.addClassDescription("Compute strain increment for small strains in 1D problems.");
  return params;
}

ADCompute1DIncrementalStrain::ADCompute1DIncrementalStrain(const InputParameters & parameters)
  : ADComputeIncrementalStrain(parameters)
{
}

void
ADCompute1DIncrementalStrain::computeTotalStrainIncrement(ADRankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation for 1D problems
  auto A = ADRankTwoTensor::initializeFromRows(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

  // Old Deformation gradient
  auto Fbar = RankTwoTensor ::initializeFromRows(
      (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

  // Compute the displacement gradient dUy/dy and dUz/dz value for 1D problems
  A(1, 1) = computeGradDispYY();
  A(2, 2) = computeGradDispZZ();

  Fbar(1, 1) = computeGradDispYYOld();
  Fbar(2, 2) = computeGradDispZZOld();

  A -= Fbar; // very nearly A = gradU - gradUold, adapted to cylindrical coords

  total_strain_increment = 0.5 * (A + A.transpose());
}
