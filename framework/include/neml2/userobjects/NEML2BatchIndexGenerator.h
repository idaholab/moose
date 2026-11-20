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

  using ElemSide = std::tuple<dof_id_type, unsigned int>;

  struct BatchSpan
  {
    std::size_t begin;
    std::size_t size;
  };

  /// Get the current batch index (in almost all cases this is the total batch size)
  std::size_t getBatchIndex() const { return _batch_index; }

  /// Whether a batch span exists for the given element ID
  bool hasBatchSpan(dof_id_type elem_id) const;

  /// Whether a batch span exists for the given element side
  bool hasSideBatchSpan(const ElemSide & elem_side) const;

  /// Get the batch span for the given element ID
  BatchSpan getBatchSpan(dof_id_type elem_id) const;

  /// Get the batch span for the given element side
  BatchSpan getSideBatchSpan(const ElemSide & elem_side) const;

  /// Whether the batch is empty
  bool isEmpty() const { return _batch_index == 0; }

protected:
  /// Whether element-interior quadrature points contribute to the batch
  const bool _volume;

  /// Whether the batch index map is outdated
  bool _outdated;

  /// Highest current batch index
  std::size_t _batch_index;

  /// Map from element IDs to batch indices
  std::map<dof_id_type, BatchSpan> _elem_to_batch_index;

  /// Map from element sides to batch spans
  std::map<ElemSide, BatchSpan> _elemside_to_batch_index;
};
