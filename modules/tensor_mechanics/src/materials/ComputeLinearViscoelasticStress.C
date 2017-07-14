/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeLinearViscoelasticStress.h"

template <>
InputParameters
validParams<ComputeLinearViscoelasticStress>()
{
  InputParameters params = validParams<ComputeLinearElasticStress>();
  params.addClassDescription("Divides total strain into elastic + creep + eigenstrains");
  params.addParam<std::string>("creep_strain_name", "creep_strain", "name of the creep strain");
  return params;
}

ComputeLinearViscoelasticStress::ComputeLinearViscoelasticStress(const InputParameters & parameters)
  : ComputeLinearElasticStress(parameters),
    _creep_strain_name(getParam<std::string>("creep_strain_name")),
    _creep_strain(getMaterialPropertyByName<RankTwoTensor>(_creep_strain_name))
{
}

void
ComputeLinearViscoelasticStress::computeQpStress()
{
  _elastic_strain[_qp] = _mechanical_strain[_qp] - _creep_strain[_qp];

  _stress[_qp] = _elasticity_tensor[_qp] * _elastic_strain[_qp];

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
