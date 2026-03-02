//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "SideUserObject.h"

#include <map>

/**
 * NEML2BatchIndexGenerator iterates over the mesh and generates a map from element ID to batch
 * index which is used by NEML2ModelExecutor for transfer data between MOOSE and NEML2.
 */
template <class Base>
class NEML2BatchIndexGeneratorTmpl : public Base
{
public:
  static InputParameters validParams();

  NEML2BatchIndexGeneratorTmpl(const InputParameters & params);

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject &) override;
  void finalize() override;

  void meshChanged() override;

  /// Get the current batch index (in almost all cases this is the total batch size)
  std::size_t getBatchIndex() const { return _batch_index; }

  /// Get the batch index for the given element ID
  template <typename B = Base>
  std::enable_if_t<std::is_same_v<B, ElementUserObject>, std::size_t>
  getBatchIndex(dof_id_type elem_id) const;

  /// Get the batch index for the given element ID and side
  template <typename B = Base>
  std::enable_if_t<std::is_same_v<B, SideUserObject>, std::size_t>
  getBatchIndex(dof_id_type elem_id, unsigned int side) const;

  /// Whether the batch is empty
  bool isEmpty() const { return _batch_index == 0; }

protected:
  using Base::_fe_problem;
  using Base::_qrule;

  /// Get the index for the current element or side
  dof_id_type currentIndex();

  /// Get the batch index for the given index
  std::size_t getBatchIndexImpl(dof_id_type idx) const;

  /// Whether the batch index map is outdated
  bool _outdated;

  /// Highest current batch index
  std::size_t _batch_index;

  /// Map from element IDs to batch indices
  std::map<dof_id_type, std::size_t> _elem_to_batch_index;

  /// cache the index for the current element
  mutable std::pair<dof_id_type, std::size_t> _elem_to_batch_index_cache;

private:
  /// Number of sides per element
  unsigned int _nside;
};

using NEML2BatchIndexGenerator = NEML2BatchIndexGeneratorTmpl<ElementUserObject>;
using NEML2BoundaryBatchIndexGenerator = NEML2BatchIndexGeneratorTmpl<SideUserObject>;
