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

/**
 * This action acts right after all material objects have been constructed.
 * It iterates over the material warehouse and calls a method on each material class
 * to trigger the resolution of all requested _optional_ material properties.
 * Resolving here means setting the material property pointer, a reference to
 * which was returned by the getOptional(AD)MaterialProperty call, to either point
 * to the requested property if it exists, or to remain a nullptr.
 */
class ResolveOptionalMaterialPropertiesAction : public Action
{
public:
  static InputParameters validParams();

  ResolveOptionalMaterialPropertiesAction(const InputParameters & params);

  virtual void act() override;
};
