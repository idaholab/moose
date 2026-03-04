//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DomainUserObject.h"

#include <map>

/**
 * NEML2BatchIndexGenerator iterates over the mesh and generates a map from element ID to batch
 * index which is used by NEML2ModelExecutor for transfer data between MOOSE and NEML2.
 */
class NEML2BatchIndexGenerator : public DomainUserObject
{
public:
  static InputParameters validParams();

  NEML2BatchIndexGenerator(const InputParameters & params);

  void initialize() override;
  void executeOnElement() override;
  void executeOnBoundary() override;
  void executeOnInterface() override;
  void threadJoin(const UserObject &) override;
  void finalize() override;
  void meshChanged() override;

  /// Get the current batch index (in almost all cases this is the total batch size)
  std::size_t getBatchIndex() const { return _batch_index; }

  /// Get the batch index for the given element ID
  std::size_t getBatchIndex(dof_id_type elem_id) const;

  /// Get the batch index for the given element ID and side
  std::size_t getBatchIndex(dof_id_type elem_id, unsigned int side) const;

  /// Check if the batch index for the given element ID exists
  bool hasBatchIndex(dof_id_type elem_id) const;

  /// Check if the batch index for the given element ID and side exists
  bool hasBatchIndex(dof_id_type elem_id, unsigned int side) const;

  /// Whether the batch is empty
  bool isEmpty() const { return _batch_index == 0; }

protected:
  /// Whether the batch index map is outdated
  bool _outdated;

  /// Highest current batch index
  std::size_t _batch_index;

  ///@{
  /// Map from element IDs to batch indices
  std::map<dof_id_type, std::size_t> _elem_to_batch_index;
  /// Map from element IDs and side to batch indices
  std::map<std::pair<dof_id_type, unsigned int>, std::size_t> _face_to_batch_index;
  ///@}

  ///@{
  /// caches for the last queried batch index
  mutable std::pair<dof_id_type, std::size_t> _elem_to_batch_index_cache;
  mutable std::pair<std::pair<dof_id_type, unsigned int>, std::size_t> _face_to_batch_index_cache;
  ///@}
};
