//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GetMortarGapUOValue.h"
#include "TestWeightedGapUserObject.h"

registerMooseObject("MooseTestApp", GetMortarGapUOValue);

InputParameters
GetMortarGapUOValue::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "weighted_gap_uo", "The name of the weighted gap user object to query for values");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

GetMortarGapUOValue::GetMortarGapUOValue(const InputParameters & parameters)
  : AuxKernel(parameters),
    _weighted_gap_uo(getUserObject<TestWeightedGapUserObject>("weighted_gap_uo"))
{
  if (!isNodal())
    paramError("variable", "The variable must be of nodal type");
}

Real
GetMortarGapUOValue::computeValue()
{
  return _weighted_gap_uo.getValue(*_current_node);
}
