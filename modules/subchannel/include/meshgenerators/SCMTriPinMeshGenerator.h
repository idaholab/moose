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
#include "libmesh/point.h"

/**
 * Class to create Pin mesh in the square lattice geometry
 */
class SCMTriPinMeshGenerator : public MeshGenerator
{
public:
  SCMTriPinMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;
  /// unheated length of the fuel Pin at the entry of the assembly
  const Real _unheated_length_entry;
  /// heated length of the fuel Pin
  const Real _heated_length;
  /// unheated length of the fuel Pin at the exit of the assembly
  const Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel pins, pitch
  const Real _pitch;
  /// number of subchannels in the x direction
  const unsigned int _n_rings;
  /// number of axial cells
  const unsigned int _n_cells;
  /// Pin nodes
  std::vector<std::vector<Node *>> _pin_nodes;
  /// block index
  const unsigned int _block_id;
  /// x-y positions of the fuel pins
  std::vector<Point> _pin_position;

public:
  static InputParameters validParams();
};
