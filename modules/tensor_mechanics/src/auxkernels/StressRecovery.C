/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressRecovery.h"

registerMooseObject("TensorMechanicsApp", StressRecovery);

template <>
InputParameters
validParams<StressRecovery>()
{
  InputParameters params = validParams<NodalPatchRecovery>();
  params.addClassDescription(
      "using nodal patch recovery to construct a smooth nodal field of stress");
  params.addRequiredParam<unsigned>("index_i", "index i of stress tensor");
  params.addRequiredParam<unsigned>("index_j", "index j of stress tensor");
  return params;
}

StressRecovery::StressRecovery(const InputParameters & parameters)
  : NodalPatchRecovery(parameters),
    _index_i(getParam<unsigned>("index_i")),
    _index_j(getParam<unsigned>("index_j")),
    _stress(getMaterialPropertyByName<RankTwoTensor>("stress"))
{
}

Real
StressRecovery::computeValue()
{
  return _stress[_qp](_index_i, _index_j);
}
