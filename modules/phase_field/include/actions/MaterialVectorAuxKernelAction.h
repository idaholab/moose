//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

class MaterialVectorAuxKernelAction : public Action
{
public:
  static InputParameters validParams();

  MaterialVectorAuxKernelAction(const InputParameters & params);

  virtual void act();

protected:
  /// number of grains to create
  const unsigned int _grain_num;

  /// base name for the auxvariables
  const std::vector<std::string> & _var_name_base;

  /// number of auxvariables
  const unsigned int _num_var;

  /// list of material properties to be used
  const std::vector<MaterialPropertyName> & _prop;

  /// number of properties
  const unsigned int _num_prop;
};
