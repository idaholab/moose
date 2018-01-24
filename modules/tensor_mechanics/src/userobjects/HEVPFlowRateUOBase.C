/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HEVPFlowRateUOBase.h"

template <>
InputParameters
validParams<HEVPFlowRateUOBase>()
{
  InputParameters params = validParams<DiscreteElementUserObject>();
  params.addParam<std::string>(
      "strength_prop_name",
      "Name of strength property: Same as strength user object specified in input file");
  params.addParam<std::string>("base_name", "Base name of tensor properties to fetch");
  params.addClassDescription("User object to evaluate flow rate");

  return params;
}

HEVPFlowRateUOBase::HEVPFlowRateUOBase(const InputParameters & parameters)
  : DiscreteElementUserObject(parameters),
    _strength_prop_name(getParam<std::string>("strength_prop_name")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strength(getMaterialPropertyByName<Real>(_strength_prop_name)),
    _pk2_prop_name(_base_name + "pk2"),
    _pk2(getMaterialPropertyByName<RankTwoTensor>(_pk2_prop_name)),
    _ce(getMaterialPropertyByName<RankTwoTensor>(_base_name + "ce"))
{
}
