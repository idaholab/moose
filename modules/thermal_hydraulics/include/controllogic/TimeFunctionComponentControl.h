//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMControl.h"

class Function;

class TimeFunctionComponentControl : public THMControl
{
public:
  TimeFunctionComponentControl(const InputParameters & parameters);

  virtual void execute();

protected:
  const std::string & _component_name;
  const std::string & _param_name;
  MooseObjectParameterName _ctrl_param_name;
  const Function & _function;

public:
  static InputParameters validParams();
};
