//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestGlobalParamSuppression.h"

registerMooseObject("MooseTestApp", TestGlobalParamSuppression);

InputParameters
TestGlobalParamSuppression::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>("suppressed_param", false, "Suppressed parameter");
  params.suppressParameter<bool>("suppressed_param");
  return params;
}

TestGlobalParamSuppression::TestGlobalParamSuppression(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
  if (isParamValid("suppressed_param") && getParam<bool>("suppressed_param"))
    mooseError("Parameter was suppressed yet the value has been changed by the user");
  if (isParamSetByUser("suppressed_param"))
    mooseError("Parameter was suppressed yet is detected as set by the user");
}
