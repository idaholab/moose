/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Compute2DIncrementalStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Compute2DIncrementalStrain>()
{
  InputParameters params = validParams<ComputeIncrementalSmallStrain>();
  params.addClassDescription("Compute strain increment for incremental strains in 2D geometries.");
  return params;
}

Compute2DIncrementalStrain::Compute2DIncrementalStrain(const InputParameters & parameters)
  : ComputeIncrementalSmallStrain(parameters)
{
}

void
Compute2DIncrementalStrain::computeTotalStrainIncrement(RankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation for 2D problems
  // Note: x_disp is the radial displacement, y_disp is the axial displacement
  RankTwoTensor A(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); // Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                     (*_grad_disp_old[1])[_qp],
                     (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

  // Compute the displacement gradient (2,2) value for plane strain, generalized plane strain, or
  // axisymmetric problems
  A(2, 2) = computeGradDispZZ();
  Fbar(2, 2) = computeGradDispZZOld();

  _deformation_gradient[_qp] = A;
  _deformation_gradient[_qp].addIa(1.0);

  A -= Fbar; // very nearly A = gradU - gradUold, adapted to cylindrical coords

  total_strain_increment = 0.5 * (A + A.transpose());
}
