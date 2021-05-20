//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMooseDoOnceOnFlag.h"

registerMooseObject("MooseTestApp", TestMooseDoOnceOnFlag);

InputParameters
TestMooseDoOnceOnFlag::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  auto do_once_flags = MooseUtils::getDefaultExecFlagEnum();
  do_once_flags.clear();
  params.addParam<ExecFlagEnum>("do_once_on", do_once_flags, "ggg");

  params.set<ExecFlagEnum>("execute_on") = MooseUtils::getDefaultExecFlagEnum().getNames();

  return params;
}

TestMooseDoOnceOnFlag::TestMooseDoOnceOnFlag(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _value(0)
{
  if (getParam<ExecFlagEnum>("do_once_on").size() != 1)
    paramError("do_once_on", "Can have one and only one flag");
}

void
TestMooseDoOnceOnFlag::execute()
{
  mooseDoOnceOnFlag(++_value, *getParam<ExecFlagEnum>("do_once_on").begin());
}

Real
TestMooseDoOnceOnFlag::getValue()
{
  return _value;
}
