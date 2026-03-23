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
 * Base class for Delaunay mesh generators applied to a surface.
 */
class SurfaceDelaunayGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  SurfaceDelaunayGeneratorBase(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override = 0;

protected:
  /**
   * Calculate the normal vector of a 2D element based the first three vertices.
   * @param elem The element for which the normal vector is to be calculated
   * @return the normal vector of the 2D element
   */
  Point elemNormal(const Elem & elem);

  /**
   * Calculate the average normal vector of a 2D mesh based on the normal vectors of its elements
   * using the element areas as weights.
   * @param mesh The mesh for which the average normal vector is to be calculated
   * @return the average normal vector of the 2D mesh
   */
  Point meshNormal2D(const MeshBase & mesh);

  /**
   * Calculate the maximum deviation of the normal vectors in a given mesh from a global average
   * normal vector.
   * @param mesh The mesh for which the maximum deviation is to be calculated
   * @param global_norm The global average normal vector
   * @return the maximum deviation of the normal vectors in the mesh from the global average normal
   * in degrees
   */
  Real meshNormalDeviation2D(const MeshBase & mesh, const Point & global_norm);

  /// Whether to use automatic desired area function
  const bool _use_auto_area_func;

  /// Background size for automatic desired area function
  const Real _auto_area_func_default_size;

  /// Background size's effective distance for automatic desired area function
  const Real _auto_area_func_default_size_dist;

  /// Maximum number of points to use for the inverse distance interpolation for automatic area function
  const unsigned int _auto_area_function_num_points;

  /// Power of the polynomial used in the inverse distance interpolation for automatic area function
  const Real _auto_area_function_power;

  /// Max angle deviation from the global average normal vector in the input mesh
  const Real _max_angle_deviation;

  /// Whether the generator should be verbose
  const bool _verbose;
};
