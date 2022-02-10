//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "MooseObjectParameterName.h"

class RealComponentParameterValuePostprocessor : public GeneralPostprocessor
{
public:
  RealComponentParameterValuePostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual Real getValue();
  virtual void execute();

protected:
  FEProblemBase & _fe_problem;
  InputParameterWarehouse & _input_parameter_warehouse;

  const std::string & _component_name;
  const std::string & _param_name;
  MooseObjectParameterName _ctrl_param_name;

  Real _value;

public:
  static InputParameters validParams();
};
