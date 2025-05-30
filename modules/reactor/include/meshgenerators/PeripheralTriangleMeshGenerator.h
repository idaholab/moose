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

/**
 * This PeripheralTriangleMeshGenerator object adds a circular peripheral region to the input mesh.
 */
class PeripheralTriangleMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  PeripheralTriangleMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of the mesh generator to get the input mesh
  const MeshGeneratorName _input_name;
  /// Radius of the peripheral region's outer circular boundary
  const Real _peripheral_ring_radius;
  /// Number of segments in the peripheral region's outer circular boundary
  const unsigned int _peripheral_ring_num_segments;
  /// Desired (maximum) triangle area
  const Real _desired_area;
  /// Desired area as a function of (x,y)
  std::string _desired_area_func;
  /// The final mesh that is generated by the subgenerators;
  std::unique_ptr<MeshBase> * _build_mesh;
};
