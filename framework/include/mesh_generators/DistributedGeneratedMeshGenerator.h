//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DISTRIBUTEDGENERATEDMESHGENERATOR_H
#define DISTRIBUTEDGENERATEDMESHGENERATOR_H

#include "MeshGenerator.h"

// Forward declarations
class DistributedGeneratedMeshGenerator;

template <>
InputParameters validParams<DistributedGeneratedMeshGenerator>();

/**
 * Mesh generated from parameters
 */
class DistributedGeneratedMeshGenerator : public MeshGenerator
{
public:
  DistributedGeneratedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  ///
  bool _verbose;

  /// The dimension of the mesh
  MooseEnum _dim;

  /// Number of elements in x, y, z direction
  unsigned int _nx, _ny, _nz;

  /// The min/max values for x,y,z component
  Real _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;

  /// The type of element to build
  ElemType _elem_type;

  /// The amount by which to bias the cells in the x,y,z directions.
  /// Must be in the range 0.5 <= _bias_x <= 2.0.
  /// _bias_x < 1 implies cells are shrinking in the x-direction.
  /// _bias_x==1 implies no bias (original mesh unchanged).
  /// _bias_x > 1 implies cells are growing in the x-direction.
  Real _bias_x, _bias_y, _bias_z;

};

#endif // DISTRIBUTEDGENERATEDMESHGENERATOR_H
