/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeLinearElasticStress_CUT_DT.h"

template<>
InputParameters validParams<ComputeLinearElasticStress_CUT_DT>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress using elasticity for small strains");
  return params;
}

ComputeLinearElasticStress_CUT_DT::ComputeLinearElasticStress_CUT_DT(const InputParameters & parameters) :
    ComputeStressBase(parameters),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain"))
{
}

void
ComputeLinearElasticStress_CUT_DT::initialSetup()
{
  if (hasMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
    mooseError("This linear elastic stress calculation only works for small strains");
}

void
ComputeLinearElasticStress_CUT_DT::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  if (_stress[_qp](1, 1) > 1) {
    throw MooseException("Wooops, stress is too big!");
  }

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
