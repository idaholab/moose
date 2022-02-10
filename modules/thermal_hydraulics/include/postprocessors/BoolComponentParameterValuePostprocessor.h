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

/**
 * Postprocessor for reading a boolean value from the control logic system
 */
class BoolComponentParameterValuePostprocessor : public GeneralPostprocessor
{
public:
  BoolComponentParameterValuePostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual Real getValue();
  virtual void execute();

protected:
  InputParameterWarehouse & _input_parameter_warehouse;
  /// The name of the component we read the value from
  const std::string & _component_name;
  /// The name of boolean parameter
  const std::string & _param_name;
  /// The name of the control parameter in MOOSE
  MooseObjectParameterName _ctrl_param_name;
  /// The value of the boolean parameter in component '_component_name'
  bool _value;

public:
  static InputParameters validParams();
};
