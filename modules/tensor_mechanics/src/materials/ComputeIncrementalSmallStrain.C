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
  InputParameters params = validParams<ComputeSmallStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for small strains.");
  params.set<bool>("stateful_displacements") = true;
  return params;
}

ComputeIncrementalSmallStrain::ComputeIncrementalSmallStrain(const InputParameters & parameters) :
    ComputeSmallStrain(parameters),
    _strain_rate(declareProperty<RankTwoTensor>(_base_name + "strain_rate")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _mechanical_strain_old(declarePropertyOld<RankTwoTensor>("mechanical_strain")),
    _total_strain_old(declarePropertyOld<RankTwoTensor>("total_strain")),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _deformation_gradient(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _eigenstrain_increment(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "stress_free_strain_increment"))
{
}

void
ComputeIncrementalSmallStrain::initQpStatefulProperties()
{
  ComputeSmallStrain::initQpStatefulProperties();

  _strain_rate[_qp].zero();
  _strain_increment[_qp].zero();
  _rotation_increment[_qp].zero();
  _rotation_increment[_qp].addIa(1.0); // this remains constant
  _deformation_gradient[_qp].zero();
  _mechanical_strain_old[_qp] = _mechanical_strain[_qp];
  _total_strain_old[_qp] = _total_strain[_qp];
}


void
ComputeIncrementalSmallStrain::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    RankTwoTensor total_strain_increment;
    computeTotalStrainIncrement(total_strain_increment);

    _strain_increment[_qp] = total_strain_increment;

    //Remove the eigenstrain increment
    _strain_increment[_qp] -= _eigenstrain_increment[_qp];

    // strain rate
    _strain_rate[_qp] = _strain_increment[_qp]/_dt;

    //Update strain in intermediate configuration: rotations are not needed
    _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
    _total_strain[_qp] = _total_strain_old[_qp] + total_strain_increment;
  }
}

void
ComputeIncrementalSmallStrain::computeTotalStrainIncrement(RankTwoTensor & total_strain_increment)
{
  //Deformation gradient
  RankTwoTensor A((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); //Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]); //Old Deformation gradient

  _deformation_gradient[_qp] = A;
  _deformation_gradient[_qp].addIa(1.0); //Gauss point deformation gradient

  A -= Fbar; // A = grad_disp - grad_disp_old

  total_strain_increment = 0.5 * (A + A.transpose());
}
