/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HEVPInternalVarRateUOBase.h"

template <>
InputParameters
validParams<HEVPInternalVarRateUOBase>()
{
  InputParameters params = validParams<DiscreteElementUserObject>();
  params.addParam<std::string>(
      "flow_rate_prop_name",
      "Name of flow rate property: Same as the flow rate user object name specified in input file");
  params.addClassDescription("User Object");
  return params;
}

HEVPInternalVarRateUOBase::HEVPInternalVarRateUOBase(const InputParameters & parameters)
  : DiscreteElementUserObject(parameters),
    _flow_rate_prop_name(getParam<std::string>("flow_rate_prop_name")),
    _flow_rate(getMaterialPropertyByName<Real>(_flow_rate_prop_name))
{
}
