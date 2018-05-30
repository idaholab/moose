//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DISTRIBUTEDGENERATEDMESH_H
#define DISTRIBUTEDGENERATEDMESH_H

#include "MooseMesh.h"

class DistributedGeneratedMesh;

template <>
InputParameters validParams<DistributedGeneratedMesh>();

/**
 * Mesh generated from parameters
 */
class DistributedGeneratedMesh : public MooseMesh
{
public:
  DistributedGeneratedMesh(const InputParameters & parameters);
  DistributedGeneratedMesh(const DistributedGeneratedMesh & /* other_mesh */) = default;

  // No copy
  DistributedGeneratedMesh & operator=(const DistributedGeneratedMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;
  virtual Real getMinInDimension(unsigned int component) const override;
  virtual Real getMaxInDimension(unsigned int component) const override;

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

#endif /* DISTRIBUTEDGENERATEDMESH_H */
