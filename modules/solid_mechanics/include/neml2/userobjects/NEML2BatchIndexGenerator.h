//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NEML2ModelInterface.h"
#include "ElementUserObject.h"

#include <map>

/**
 * NEML2BatchIndexGenerator iterates over the mesh and generates a map from element ID to batch
 * index which is used by NEML2ModelExecutor for transfer data between MOOSE and NEML2.
 */
class NEML2BatchIndexGenerator : public ElementUserObject
{
public:
  static InputParameters validParams();

  NEML2BatchIndexGenerator(const InputParameters & params);

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject &) override;
  void finalize() override {}

  /// Get the batch index for the given element ID
  std::size_t getBatchIndex(dof_id_type elem_id) const;

protected:
  /// Highest current batch index
  std::size_t _batch_index;

  /// Map from element IDs to batch indices
  std::map<dof_id_type, std::size_t> _elem_to_batch_index;

  /// cache the index for the current element
  mutable std::pair<dof_id_type, std::size_t> _elem_to_batch_index_cache;
};
