/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "MeshGenerator.h"
#include "libmesh/point.h"
#include "QuadSubChannelMeshGenerator.h"

/**
 * Class to create Pin mesh in the square lattice geometry
 */
class QuadPinMeshGenerator : public MeshGenerator
{
public:
  QuadPinMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;
  /// unheated length of the fuel rod at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel rod
  Real _heated_length;
  /// unheated length of the fuel rod at the exit of the assembly
  Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel rods, pitch
  Real _pitch;
  /// number of subchannels in the x direction
  unsigned int _nx;
  /// number of subchannels in the y direction
  unsigned int _ny;
  /// number of axial cells
  unsigned int _n_cells;
  /// Pin nodes
  std::vector<std::vector<Node *>> _pin_nodes;
  /// block index
  unsigned int _block_id;

public:
  static InputParameters validParams();
};
