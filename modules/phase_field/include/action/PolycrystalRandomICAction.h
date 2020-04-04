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
#include "InputParameters.h"
#include "MooseEnum.h"

/**
 * Automatically generates all variables to model a polycrystal with op_num orderparameters
 */
class PolycrystalRandomICAction : public Action
{
public:
  static InputParameters validParams();

  PolycrystalRandomICAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _op_num;
  const std::string _var_name_base;
  const MooseEnum _random_type;
};
