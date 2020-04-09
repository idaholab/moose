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

class CavityPressureAction : public Action
{
public:
  static InputParameters validParams();

  CavityPressureAction(const InputParameters & params);

  virtual void act() override;

  /// Flag to use automatic differentiation where possible
  const bool _use_ad;
};
