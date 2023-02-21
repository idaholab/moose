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
#include "MooseEnum.h"

/*
 * Mesh generator to create a Cartesian mesh
 */
class BlockCartesianGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  BlockCartesianGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;

  /// Number of elements in x, y, z direction
  dof_id_type &_nx, &_ny, &_nz;

  /// Number of cores for partitioning the graph
  processor_id_type _num_cores_for_partition;

  /// The type of element to build
  ElemType _elem_type;

  /// The amount by which to bias the cells in the x,y,z directions.
  /// Must be in the range 0.5 <= _bias_x <= 2.0.
  /// _bias_x < 1 implies cells are shrinking in the x-direction.
  /// _bias_x==1 implies no bias (original mesh unchanged).
  /// _bias_x > 1 implies cells are growing in the x-direction.
  Real _bias_x, _bias_y, _bias_z;

  /// External partitioner
  std::string _part_package;

  /// Number of cores per compute node if hierarch partitioning is used
  processor_id_type _num_parts_per_compute_node;

  /// Which method is used to partition the mesh that is not built yet
  std::string _partition_method;

  /// Number of element side neighbor layers
  /// While most of applications in moose require one layer of side neighbors,
  /// phase field simulation with grain tracker needs two layers. This parameter
  /// allow us to reserve an arbitrary number of side neighbors
  unsigned _num_side_layers;
  /// Name of the generated Cartesian mesh
  std::unique_ptr<MeshBase> * _build_mesh;
  /// Name of the base mesh
  std::unique_ptr<MeshBase> & _mesh_input;
};
