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
  AddRayTracingObjectAction(const InputParameters & params);

  static InputParameters validParams();

  void act() override final;

protected:
  /**
   * Calls addObject on the problem to add the desired object after
   * _ray_tracing_study is set in _moose_object_pars
   */
  virtual void addRayTracingObject() = 0;
};
