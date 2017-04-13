/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeLinearElasticStress.h"

template <>
InputParameters
validParams<ComputeLinearElasticStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress using elasticity for small strains");
  return params;
}

ComputeLinearElasticStress::ComputeLinearElasticStress(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain"))
{
}

void
ComputeLinearElasticStress::initialSetup()
{
  if (hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
    mooseError("This linear elastic stress calculation only works for small strains; use "
               "ComputeFiniteStrainElasticStress for simulations using incremental and finite "
               "strains.");
}

void
ComputeLinearElasticStress::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
