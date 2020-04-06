//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

/**
 * Mesh generated from parameters
 */
class GeneratedMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  GeneratedMesh(const InputParameters & parameters);
  GeneratedMesh(const GeneratedMesh & /* other_mesh */) = default;

  // No copy
  GeneratedMesh & operator=(const GeneratedMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;
  virtual Real getMinInDimension(unsigned int component) const override;
  virtual Real getMaxInDimension(unsigned int component) const override;
  virtual void prepared(bool state) override;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;

  /// Number of elements in x, y, z direction
  unsigned int _nx, _ny, _nz;

  /// The min/max values for x,y,z component
  Real _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;

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

  /// Boolean to indicate that dimensions may have changed
  bool _dims_may_have_changed;
};
