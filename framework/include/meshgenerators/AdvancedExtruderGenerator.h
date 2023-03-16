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

#include "libmesh/point.h"

/**
 * Extrudes a mesh to another dimension
 */
class AdvancedExtruderGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  AdvancedExtruderGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// Height of each elevation
  const std::vector<Real> & _heights;

  /// Bias growth factor of each elevation
  const std::vector<Real> _biases;

  /// Number of layers in each elevation
  const std::vector<unsigned int> & _num_layers;

  /// Subdomains to swap out for each elevation
  const std::vector<std::vector<subdomain_id_type>> & _subdomain_swaps;

  /// Boundaries to swap out for each elevation
  const std::vector<std::vector<boundary_id_type>> & _boundary_swaps;

  /// Names and indices of extra element integers to swap
  const std::vector<std::string> & _elem_integer_names_to_swap;
  std::vector<unsigned int> _elem_integer_indices_to_swap;

  /// Extra element integers to swap out for each elevation and each element interger name
  const std::vector<std::vector<std::vector<dof_id_type>>> & _elem_integers_swaps;

  /// Easier to work with version of _sudomain_swaps
  std::vector<std::unordered_map<subdomain_id_type, subdomain_id_type>> _subdomain_swap_pairs;

  /// Easier to work with version of _boundary_swaps
  std::vector<std::unordered_map<boundary_id_type, boundary_id_type>> _boundary_swap_pairs;

  /// Easier to work with version of _elem_integers_swaps
  std::vector<std::unordered_map<unsigned int, unsigned int>> _elem_integers_swap_pairs;

  /// The direction of the extrusion
  Point _direction;

  bool _has_top_boundary;
  boundary_id_type _top_boundary;

  bool _has_bottom_boundary;
  boundary_id_type _bottom_boundary;

  /// The list of input mesh's blocks that need to be assigned upward boundary interfaces for each layer of elevation
  const std::vector<std::vector<subdomain_id_type>> _upward_boundary_source_blocks;

  /// Upward boundary interfaces for each layer of elevation
  const std::vector<std::vector<boundary_id_type>> _upward_boundary_ids;

  /// The list of input mesh's blocks that need to be assigned downward boundary interfaces for each layer of elevation
  const std::vector<std::vector<subdomain_id_type>> _downward_boundary_source_blocks;

  /// Downward boundary interfaces for each layer of elevation
  const std::vector<std::vector<boundary_id_type>> _downward_boundary_ids;

  /**
   * Swap two nodes within an element
   * @param elem element whose nodes need to be swapped
   * @param nd1 index of the first node to be swapped
   * @param nd2 index of the second node to be swapped
   */
  void swapNodesInElem(Elem & elem, const unsigned int nd1, const unsigned int nd2);
};
