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

private:
  void setupMesh(MooseMesh * mesh);

  /**
   * Modifies the MooseObject's parameters to build the right type of Mesh when using splits.
   * @return The new type of object that will be built.
   */
  std::string modifyParamsForUseSplit(InputParameters & moose_object_params) const;

  /// Whether or not to use a split mesh; comes from either Mesh/use_split or --use-split
  const bool _use_split;
  /// The split mesh file (if any); comes from either Mesh/split_file or --split-file
  const std::string _split_file;
};
