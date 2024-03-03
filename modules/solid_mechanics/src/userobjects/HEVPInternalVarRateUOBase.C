//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPInternalVarRateUOBase.h"

InputParameters
HEVPInternalVarRateUOBase::validParams()
{
  InputParameters params = DiscreteElementUserObject::validParams();
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
