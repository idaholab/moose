//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidProperties.h"
#include "MooseObject.h"

InputParameters
SolidProperties::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>(
      "allow_imperfect_jacobians",
      false,
      "true to allow unimplemented property derivative terms to be set to zero for the AD API");
  params.registerBase("SolidProperties");

  // Suppress unused parameters
  params.suppressParameter<bool>("use_displaced_mesh");
  params.suppressParameter<ExecFlagEnum>("execute_on");
  params.suppressParameter<bool>("allow_duplicate_execution_on_initial");
  params.suppressParameter<bool>("force_preic");
  params.suppressParameter<bool>("force_preaux");
  params.suppressParameter<bool>("force_postaux");
  params.suppressParameter<int>("execution_order_group");

  return params;
}

SolidProperties::SolidProperties(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    SolutionInvalidInterface(this),
    _allow_imperfect_jacobians(getParam<bool>("allow_imperfect_jacobians"))
{
}
