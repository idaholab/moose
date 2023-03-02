//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidProperties.h"

const Real FluidProperties::_R = 8.3144598;

InputParameters
FluidProperties::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>(
      "allow_imperfect_jacobians",
      false,
      "true to allow unimplemented property derivative terms to be set to zero for the AD API");
  params.addCustomTypeParam<std::string>("fp_type", "FPType", "Type of the fluid property object");
  params.addParamNamesToGroup("fp_type allow_imperfect_jacobians", "Advanced");
  params.registerBase("FluidProperties");
  return params;
}

FluidProperties::FluidProperties(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _T_c2k(273.15),
    _allow_imperfect_jacobians(getParam<bool>("allow_imperfect_jacobians"))
{
}

FluidProperties::~FluidProperties() {}
