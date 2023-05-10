//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Positions.h"
#include "BlockRestrictable.h"

/**
 * Positions from centroids of groups of elements (subdomains, extra element ids) in the mesh
 */
class ElementGroupCentroidPositions : public Positions, public BlockRestrictable
{
public:
  static InputParameters validParams();
  ElementGroupCentroidPositions(const InputParameters & parameters);
  virtual ~ElementGroupCentroidPositions() = default;

  void initialize() override;

  MooseMesh & _mesh;

  ///{
  /// How to group elements, to compute centroids for these groups
  static MooseEnum groupTypeEnum()
  {
    return MooseEnum("block extra_id block_and_extra_id", "block");
  }
  MooseEnum _group_type;
  ///}

  /// Whether blocks are used for the outermost indexing
  bool _blocks_in_use;

  /// The names of the extra element ids to use to make groups
  std::vector<ExtraElementIDName> _extra_id_names;
  /// The indices of the extra element ids to use to make groups
  std::vector<unsigned int> _extra_id_indices;
  /// The particular extra id values, for each extra id of interest
  std::vector<std::vector<unsigned int>> _extra_id_group_indices;

private:
  unsigned int id(const Elem & elem, unsigned int id_index, bool use_subdomains);
};
