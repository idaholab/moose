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

namespace MeshTriangulationUtils
{
struct XYDelaunayOptions;
}

/**
 * Base class for Delaunay mesh generators applied to a surface.
 */
class SurfaceDelaunayGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  /**
   * The parameters that select the outer boundary to triangulate within and the holes to leave out
   * of the triangulation, together with the names and the element area limit the result gets. A
   * generator that takes its region to mesh as a boundary mesh and hole meshes adds these to its
   * own parameters; the ones derived from this class that select that region some other way do
   * not, which is why the base class does not add them itself.
   * @return the parameters shared by the generators that triangulate within an input boundary mesh
   */
  static InputParameters boundaryAndHolesParams();

  SurfaceDelaunayGeneratorBase(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override = 0;

protected:
  /**
   * Errors if the parameters boundaryAndHolesParams() adds contradict each other or the holes they
   * refer to. Only for a generator that added those parameters.
   * @param hole_ptrs The input meshes 'holes' resolved to
   */
  void
  checkBoundaryAndHolesParams(const std::vector<std::unique_ptr<MeshBase> *> & hole_ptrs) const;

  /**
   * Errors if a point was given twice as an interior point, which the triangulation cannot honor.
   * @param interior_points The interior points to be meshed through
   */
  void checkInteriorPoints(const std::vector<Point> & interior_points) const;

  /**
   * Fills the triangulation options that follow from the parameters boundaryAndHolesParams() adds,
   * and only those: the caller fills whatever its own parameters set on top of them.
   * @param opts The options to fill
   */
  void fillDelaunayOptions(MeshTriangulationUtils::XYDelaunayOptions & opts) const;

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
