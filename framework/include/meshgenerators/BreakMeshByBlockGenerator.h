//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BreakMeshByBlockGeneratorBase.h"
#include <unordered_set>

class BreakMeshByBlockGenerator;

template <>
InputParameters validParams<BreakMeshByBlockGenerator>();

/*
 * A mesh generator to split a mesh by a set of blocks
 */
class BreakMeshByBlockGenerator : public BreakMeshByBlockGeneratorBase
{
public:
  static InputParameters validParams();

  BreakMeshByBlockGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// This is a helper method to avoid recoding the same if everywhere.
  /// If this mesh modifier is used in block restricted mode and the provided
  /// element belongs to one of the provided blocks it returns the
  /// element subdomain id, an invalid subdomain id otherwise.
  /// If this mesh modifier is not block restricted, then the method always
  /// returns the element subdomain id.
  /// Notice that in block restricted mode, the invalid_subdomain_id is used
  /// to lump toghether all the non-listed blocks to avoid splitting the mesh
  /// where not necessary.
  subdomain_id_type blockRestrictedElementSubdomainID(const Elem * elem);

  /// the mesh to modify
  std::unique_ptr<MeshBase> & _input;
  /// the blocks to split the mesh on
  const std::vector<SubdomainID> _block;
  /// set of the blocks to split the mesh on
  const std::unordered_set<SubdomainID> _block_set;
  /// whether a subset of blocks should be split away or all of them
  const bool _block_restricted;
  /// whether to add a boundary when splitting the mesh
  const bool _add_transition_interface;
  /// whether to split the transition boundary between the blocks and the rest of the mesh
  const bool _split_transition_interface;
  /// the name of the transition interface
  const BoundaryName _interface_transition_name;

private:
  /// generate the new boundary interface
  void addInterfaceBoundary(MeshBase & mesh);

  std::set<std::pair<subdomain_id_type, subdomain_id_type>> _neighboring_block_list;
  std::map<std::pair<subdomain_id_type, subdomain_id_type>,
           std::set<std::pair<dof_id_type, unsigned int>>>
      _new_boundary_sides_map;
};
