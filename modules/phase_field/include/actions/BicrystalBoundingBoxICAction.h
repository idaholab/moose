//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "Action.h"

/**
 * Bicrystal using a bounding box
 */
class BicrystalBoundingBoxICAction : public Action
{
public:
  static InputParameters validParams();

  BicrystalBoundingBoxICAction(const InputParameters & params);

  virtual void act();

private:
  const std::string _var_name_base;
  const unsigned int _op_num;
};
