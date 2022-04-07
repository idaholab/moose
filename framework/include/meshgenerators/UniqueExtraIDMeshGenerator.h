//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
/**
 * Add a new extra ID by finding unique combinations of existing extra ID values
 */
class UniqueExtraIDMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  UniqueExtraIDMeshGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// input mesh for adding element IDs
  std::unique_ptr<MeshBase> & _input;
  /// existing extra ID names to be used for adding a new extra ID
  const std::vector<ExtraElementIDName> _extra_ids;
  /// lag to indicate if new_id_rule is defined
  const bool _use_new_id_rule;
};
