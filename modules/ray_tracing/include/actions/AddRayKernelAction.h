//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AddRayTracingObjectAction.h"

/**
 * Action for creating a RayKernel and associating it with the necessary RayTracingStudy objects.
 */
class AddRayKernelAction : public AddRayTracingObjectAction
{
public:
  AddRayKernelAction(const InputParameters & params);

  static InputParameters validParams();

protected:
  void addRayTracingObject() override final;
};
