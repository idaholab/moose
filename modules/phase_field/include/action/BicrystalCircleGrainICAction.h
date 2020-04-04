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
 * Bicrystal with a circular grain and an embedding outer grain
 */
class BicrystalCircleGrainICAction : public Action
{
public:
  static InputParameters validParams();

  BicrystalCircleGrainICAction(const InputParameters & params);

  virtual void act();

private:
  const std::string _var_name_base;
  const unsigned int _op_num;

  const Real _radius;
  const Real _x, _y, _z;
  const Real _int_width;

  const bool _3D_sphere;
};
