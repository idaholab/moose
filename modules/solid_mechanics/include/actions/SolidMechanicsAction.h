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
#include "MooseTypes.h"

class SolidMechanicsAction;

template <>
InputParameters validParams<SolidMechanicsAction>();

class SolidMechanicsAction : public Action
{
public:
  SolidMechanicsAction(const InputParameters & params);

  virtual void act();

private:
  const VariableName _disp_x;
  const VariableName _disp_y;
  const VariableName _disp_z;
  const VariableName _disp_r;
  const VariableName _temp;
  const Real _zeta;
  const Real _alpha;
};

