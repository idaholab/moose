/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeCosseratLinearElasticStress.h"

template <>
InputParameters
validParams<ComputeCosseratLinearElasticStress>()
{
  InputParameters params = validParams<ComputeCosseratStressBase>();
  params.addClassDescription(
      "Compute Cosserat stress and couple-stress elasticity for small strains");
  return params;
}

ComputeCosseratLinearElasticStress::ComputeCosseratLinearElasticStress(
    const InputParameters & parameters)
  : ComputeCosseratStressBase(parameters)
{
}

void
ComputeCosseratLinearElasticStress::initialSetup()
{
  if (hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
    mooseError("This linear elastic stress calculation only works for small strains");
}

void
ComputeCosseratLinearElasticStress::computeQpStress()
{
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
  _stress_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp] * _curvature[_qp];

  _elastic_strain[_qp] = _mechanical_strain[_qp];

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  _Jacobian_mult_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp];
}
