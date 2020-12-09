//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"

/**
 * Base class for adding a RayTracingObject and associating it with the necessary RayTracingStudy
 */
class AddRayTracingObjectAction : public MooseObjectAction
{
public:
  AddRayTracingObjectAction(InputParameters params);

  static InputParameters validParams();

  virtual void act();

private:
  /**
   * Gets a RayTracingStudy to associate with a RayTracingObject and sets it as the
   * '_ray_tracing_study' private parameter.
   */
  void setRayTracingStudy();
};
