//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include <unordered_set>

/*
 * A mesh generator to split a mesh by a set of blocks
 */
class BreakMeshByBlockGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  BreakMeshByBlockGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

private:
  /// check that if split_interface==true interface_id and interface_name are
  /// not set by the user. It also check that the provided interface_id is not
  /// already used
  void checkInputParameter();

  /// given the primary and secondary blocks this method return the appropriate
  /// boundary id and name
  void findBoundaryName(const MeshBase & mesh,
                        subdomain_id_type primaryBlockID,
                        subdomain_id_type secondaryBlockID,
                        BoundaryName & boundary_name,
                        boundary_id_type boundaryID,
                        BoundaryInfo & boundary_info);

  /// A container holding (boundary name, boundary ID) associations.
  std::set<std::pair<std::string, BoundaryID>> _bName_bID_set;

  /// this method generate the boundary name by assembling subdomain names
  BoundaryName generateBoundaryName(const MeshBase & mesh,
                                    subdomain_id_type primaryBlockID,
                                    subdomain_id_type secondaryBlockID);

  /// this method save the boundary name/id pair
  void mapBoundaryIdAndBoundaryName(boundary_id_type boundaryID, const std::string & boundaryName);

  /// generate the new boundary interface
  void addInterface(MeshBase & mesh);

  /**
   * @brief Synchronizes connected blocks across all MPI ranks.
   *
   * This process consists of two phases:
   * Phase 0: Each rank computes the locally connected blocks for the nodes it owns and sends
   *          this information to the owner of each node.
   * Phase 1: The owner of each node aggregates all received connected block information and
   *          broadcasts the global set of connected blocks for each node to all ranks.
   */
  std::unordered_map<dof_id_type, std::set<subdomain_id_type>> syncConnectedBlocks(
      const std::unordered_map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map,
      const MeshBase & mesh);

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
  /// the name of the new interface
  std::string _interface_name;
  /// the flag to split the interface by block
  bool _split_interface;
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

  using NodeConnectedBlocksPair = std::pair<dof_id_type, std::vector<subdomain_id_type>>;
  using SubdomainPair = std::pair<subdomain_id_type, subdomain_id_type>;
  using ElemSide = std::tuple<const Elem *, unsigned int>;

  /// Set of pairs of block ids between which new boundary sides are created
  std::set<SubdomainPair> _neighboring_block_list;

  /// Map from a pair of block ids to a set of ElemSide tuples. Each tuple
  /// contains an element pointer and a side index.
  std::unordered_map<SubdomainPair, std::set<ElemSide>> _subid_pairs_to_sides;

  /// Map from a pair of block ids to the corresponding boundary id.
  std::unordered_map<std::pair<subdomain_id_type, subdomain_id_type>, boundary_id_type>
      _subid_pairs_to_boundary_id;
};
