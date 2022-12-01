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

class MooseMesh;

class SetupMeshAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  SetupMeshAction(const InputParameters & params);

  virtual void act() override;

protected:
  /**
   * Executes this action's 'setup_mesh' task
   *
   * @param[in] mesh_generator_mesh_type  Type of mesh generator mesh
   */
  void executeSetupMeshTask(const std::string & mesh_generator_mesh_type);

  /**
   * Executes this action's 'set_mesh_base' task
   */
  void executeSetMeshBaseTask();

  /**
   * Executes this action's 'init_mesh' task
   */
  void executeInitMeshTask();

private:
  void setupMesh(MooseMesh * mesh);

  /**
   * Modifies the MooseObject's parameters to build the right type of Mesh when using splits.
   * @return The new type of object that will be built.
   */
  std::string modifyParamsForUseSplit(InputParameters & moose_object_params) const;
};
