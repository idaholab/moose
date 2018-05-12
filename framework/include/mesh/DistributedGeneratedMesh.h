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

  virtual MooseMesh & clone() const override;
  virtual void buildMesh() override;
  virtual Real getMinInDimension(unsigned int component) const override;
  virtual Real getMaxInDimension(unsigned int component) const override;

protected:
  /*
  void distributed_build_line(UnstructuredMesh & mesh,
                              const unsigned int nx,
                              const Real xmin = 0.,
                              const Real xmax = 1.,
                              const ElemType type = INVALID_ELEM,
                              const bool gauss_lobatto_grid = false);

  void build_square(UnstructuredMesh & mesh,
                    const unsigned int nx,
                    const unsigned int ny,
                    const Real xmin = 0.,
                    const Real xmax = 1.,
                    const Real ymin = 0.,
                    const Real ymax = 1.,
                    const ElemType type = INVALID_ELEM,
                    const bool gauss_lobatto_grid = false);
  */

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

  /// All of the libmesh build_line/square/cube routines support an
  /// option to grade the mesh into the boundaries according to the
  /// spacing of the Gauss-Lobatto quadrature points.  Defaults to
  /// false, and cannot be used in conjunction with x, y, and z
  /// biasing.
  bool _gauss_lobatto_grid;

  /// The amount by which to bias the cells in the x,y,z directions.
  /// Must be in the range 0.5 <= _bias_x <= 2.0.
  /// _bias_x < 1 implies cells are shrinking in the x-direction.
  /// _bias_x==1 implies no bias (original mesh unchanged).
  /// _bias_x > 1 implies cells are growing in the x-direction.
  Real _bias_x, _bias_y, _bias_z;
};

#endif /* DISTRIBUTEDGENERATEDMESH_H */
