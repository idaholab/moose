/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeCosseratStressBase.h"

template <>
InputParameters
validParams<ComputeCosseratStressBase>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress and couple stress in the Cosserat situation");
  return params;
}

ComputeCosseratStressBase::ComputeCosseratStressBase(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _curvature(getMaterialPropertyByName<RankTwoTensor>("curvature")),
    _elastic_flexural_rigidity_tensor(
        getMaterialPropertyByName<RankFourTensor>("elastic_flexural_rigidity_tensor")),
    _stress_couple(declareProperty<RankTwoTensor>("couple_stress")),
    _Jacobian_mult_couple(declareProperty<RankFourTensor>("couple_Jacobian_mult"))
{
  if (_store_stress_old)
  {
    declarePropertyOld<RankTwoTensor>("couple_stress");
    declarePropertyOlder<RankTwoTensor>("couple_stress");
  }
}

void
ComputeCosseratStressBase::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
  _stress_couple[_qp].zero();
}
