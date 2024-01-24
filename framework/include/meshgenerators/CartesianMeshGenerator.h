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
class CartesianMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  CartesianMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;
  /// Intervals in x direction
  std::vector<Real> _dx;
  /// Number of grids in all intervals in x direction
  std::vector<unsigned int> _ix;
  /// Intervals in y direction
  std::vector<Real> _dy;
  /// Number of grids in all intervals in y direction
  std::vector<unsigned int> _iy;
  /// Intervals in z direction
  std::vector<Real> _dz;
  /// Number of grids in all intervals in z direction
  std::vector<unsigned int> _iz;
  /// Block IDs
  std::vector<unsigned int> _subdomain_id;
  /// Number of elements in x, y, z direction
  int _nx, _ny, _nz;
};
