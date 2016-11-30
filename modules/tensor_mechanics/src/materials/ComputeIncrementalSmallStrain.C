/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeIncrementalSmallStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ComputeIncrementalSmallStrain>()
{
  InputParameters params = validParams<ComputeIncrementalStrainBase>();
  params.addClassDescription("Compute a strain increment and rotation increment for small strains.");
  return params;
}

ComputeIncrementalSmallStrain::ComputeIncrementalSmallStrain(const InputParameters & parameters) :
    ComputeIncrementalStrainBase(parameters)
{
}

void
ComputeIncrementalSmallStrain::computeQpProperties()
{
  RankTwoTensor total_strain_increment;
  computeTotalStrainIncrement(total_strain_increment);

  _strain_increment[_qp] = total_strain_increment;

  //Remove the eigenstrain increment
  subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

  // strain rate
  if (_dt > 0)
    _strain_rate[_qp] = _strain_increment[_qp] /_dt;
  else
    _strain_rate[_qp].zero();

  //Update strain in intermediate configuration: rotations are not needed
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
  _total_strain[_qp] = _total_strain_old[_qp] + total_strain_increment;
}

void
ComputeIncrementalSmallStrain::computeTotalStrainIncrement(RankTwoTensor & total_strain_increment)
{
  //Deformation gradient
  RankTwoTensor A((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); //Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]); //Old Deformation gradient

  _deformation_gradient[_qp] = A;
  _deformation_gradient[_qp].addIa(1.0);

  A -= Fbar; // A = grad_disp - grad_disp_old

  total_strain_increment = 0.5 * (A + A.transpose());
}
