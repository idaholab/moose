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
 * Sets up a polycrystal initial condition with voids on grain boundaries for
 * all order parameters.
 */
class PolycrystalVoronoiVoidICAction : public Action
{
public:
  static InputParameters validParams();

  PolycrystalVoronoiVoidICAction(const InputParameters & params);

  virtual void act();

protected:
  const unsigned int _op_num;
  const std::string _var_name_base;
  const FileName _file_name;
};
