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

class BreakMeshByBlockGenerator : public BreakMeshByBlockGeneratorBase
{
public:
  static InputParameters validParams();

  BreakMeshByBlockGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// This is a helper method to avoid recoding the same if everywhere.
  /// If this mesh modifier is used in block restricted mode and the provide
  /// element belongs to one of the provided blocks it returns the
  /// element subdomain id, an invalid subdomain id otherwise.
  /// If this mesh modifier is not block restricted, then the method always
  /// returns the element subdomain id.
  /// Notice that in block restricted mode, the invalid_subdomain_id is used
  /// to lump toghether all the non-listed blocks to avoid splitting the mesh
  /// where not necessary.
  subdomain_id_type blockRestrictedElementSubdomainID(const Elem * elem);

  std::unique_ptr<MeshBase> & _input;
  const std::vector<SubdomainID> _block;
  const std::unordered_set<SubdomainID> _block_set;
  const bool _block_restricted;
  const bool _add_transition_interface;
  const bool _split_transition_interface;
  const BoundaryName _interface_transition_name;
  const std::string _integer_name = "bmbb_element_id";

private:
  /// generate the new boundary interface
  void addInterfaceBoundary(MeshBase & mesh);
  /// method writing teh fake neighbor conectivity to file
  void writeFakeNeighborListToFile(MeshBase & mesh) const;

  std::set<std::pair<subdomain_id_type, subdomain_id_type>> _neighboring_block_list;
  std::map<std::pair<subdomain_id_type, subdomain_id_type>,
           std::set<std::pair<dof_id_type, unsigned int>>>
      _new_boundary_sides_map;

  /// flag allwoing to write the fake neighbor list to file
  const bool _write_fake_neighbor_list_to_file;

  /// the filename where the fake neighbor list will be saved
  const FileName _fake_neighbor_list_file_name;
};
