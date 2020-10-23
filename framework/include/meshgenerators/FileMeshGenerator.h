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

// Forward declarations
class FileMeshGenerator;

template <>
InputParameters validParams<FileMeshGenerator>();

/**
 * Generates a mesh by reading it from an file.
 */
class FileMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FileMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// wrapper method used to reassign fake neighbors
  void reassignFakeNeighbors(MeshBase & mesh);

  // read the fake neighbor list from the provided csv file and generates the direct and inverse
  // fake neighbor maps
  void readFakeNeighborListFromFile();

  /// search in the map
  dof_id_type getRealIDFromInteger(const unsigned int elem_integer) const;

  /// actually modfying the mesh adding fake neighbors links
  bool assignFakeNeighbors(
      MeshBase & mesh,
      Elem & elem,
      const std::pair<unsigned int, unsigned int> & elem_side,
      const std::map<std::pair<dof_id_type, unsigned int>, std::pair<dof_id_type, unsigned int>> &
          fake_neighbor_map) const;

  const MeshFileName & _file_name;
  const bool _has_fake_neighbors;
  const FileName _fake_neighbor_list_file_name;

  std::map<std::pair<dof_id_type, unsigned int>, std::pair<dof_id_type, unsigned int>>
      _fake_neighbor_map;
  std::map<std::pair<dof_id_type, unsigned int>, std::pair<dof_id_type, unsigned int>>
      _fake_neighbor_map_inverted;
  std::map<unsigned int, dof_id_type> _uelemid_to_elemid;
  unsigned int _integer_id;
};
