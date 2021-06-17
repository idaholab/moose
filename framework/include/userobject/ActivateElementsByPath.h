//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActivateElementsUserObjectBase.h"
#include "Function.h"

class ActivateElementsByPath : public ActivateElementsUserObjectBase
{
public:
  static InputParameters validParams();

  ActivateElementsByPath(const InputParameters & parameters);

  virtual bool isElementActivated() override;

protected:
  /// path of the heat source, x, y, z components
  const Function * _function_x;
  const Function * _function_y;
  const Function * _function_z;
  /// define the distance of the element to the point on the path,
  /// below which the element will be activated
  const Real _activate_distance;
};
