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

  /// Return true if block_one and block_two are found in users' provided block_pairs list
  bool findBlockPairs(subdomain_id_type block_one, subdomain_id_type block_two);

  /// the mesh to modify
  std::unique_ptr<MeshBase> & _input;
  /// set of subdomain pairs between which interfaces will be generated.
  std::unordered_set<std::pair<SubdomainID, SubdomainID>> _block_pairs;
  /// set of the blocks to split the mesh on
  std::unordered_set<SubdomainID> _block_set;
  /// whether interfaces will be generated between block pairs
  const bool _block_pairs_restricted;
  /// whether interfaces will be generated surrounding blocks
  const bool _surrounding_blocks_restricted;
  /// whether to add a boundary when splitting the mesh
  const bool _add_transition_interface;
  /// whether to split the transition boundary between the blocks and the rest of the mesh
  const bool _split_transition_interface;
  /// the name of the transition interface
  const BoundaryName _interface_transition_name;
  /// whether to add two sides interface boundaries
  const bool _add_interface_on_two_sides;

private:
  /// generate the new boundary interface
  void addInterfaceBoundary(MeshBase & mesh);

  std::set<std::pair<subdomain_id_type, subdomain_id_type>> _neighboring_block_list;
  std::map<std::pair<subdomain_id_type, subdomain_id_type>,
           std::set<std::pair<dof_id_type, unsigned int>>>
      _new_boundary_sides_map;
};
