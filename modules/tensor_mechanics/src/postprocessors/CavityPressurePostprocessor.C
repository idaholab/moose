//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CavityPressurePostprocessor.h"

#include "CavityPressureUserObject.h"

template <>
InputParameters
validParams<CavityPressurePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription("Interfaces with the CavityPressureUserObject to store"
                             "the initial number of moles of a gas contained within"
                             "an internal volume");
  params.addRequiredParam<UserObjectName>(
      "cavity_pressure_uo", "The CavityPressureUserObject that computes the initial moles");
  params.addRequiredParam<std::string>("quantity", "The quantity to report");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

CavityPressurePostprocessor::CavityPressurePostprocessor(const InputParameters & params)
  : GeneralPostprocessor(params),
    _cpuo(getUserObject<CavityPressureUserObject>("cavity_pressure_uo")),
    _quantity(getParam<std::string>("quantity"))
{
}

PostprocessorValue
CavityPressurePostprocessor::getValue()
{
  return _cpuo.getValue(_quantity);
}
