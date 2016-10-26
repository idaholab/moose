/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Compute1DIncrementalStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<Compute1DIncrementalStrain>()
{
  InputParameters params = validParams<ComputeIncrementalSmallStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains in 2D geometries.");
  return params;
}

Compute1DIncrementalStrain::Compute1DIncrementalStrain(const InputParameters & parameters) :
    ComputeIncrementalSmallStrain(parameters)
{
}

void
Compute1DIncrementalStrain::computeTotalStrainIncrement(RankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation for 1D problems
  // Note: x_disp is the radial displacement
  RankTwoTensor A((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); //Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]); //Old Deformation gradient

  // Compute the deformation gradient value for plane strain, generalized plane strain, or axisymmetric problems
  A(1,1) = computeDUYDY();
  Fbar(1,1) = computeDUYDYOld();
  A(2,2) = computeDUZDZ();
  Fbar(2,2) = computeDUZDZOld();

  // Gauss point deformation gradient
  _deformation_gradient[_qp] = A;
  _deformation_gradient[_qp].addIa(1.0);

  A -= Fbar; //very nearly A = gradU - gradUold, adapted to cylindrical coords

  total_strain_increment = 0.5 * (A + A.transpose());
}
