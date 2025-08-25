//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurfaceDelaunayGeneratorBase.h"
#include "FunctionParserUtils.h"

#include "libmesh/meshfree_interpolation.h"

class Boundary2DDelaunayGenerator : public SurfaceDelaunayGeneratorBase,
                                    public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  Boundary2DDelaunayGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  ///The input mesh name
  std::unique_ptr<MeshBase> & _input;

  ///The boundaries to be converted to a 2D mesh
  const std::vector<BoundaryName> _boundary_names;

  ///The boundaries to be used as holes
  const std::vector<std::vector<BoundaryName>> _hole_boundary_names;

  /// Maximum number of iterations to correct the nodes based on the level set function
  const unsigned int _max_level_set_correction_iterations;

  /// Max angle deviation from the global average normal vector in the input mesh
  const Real _max_angle_deviation;

  /// The name of the external boundary of the mesh to generate
  BoundaryName _output_external_boundary_name;

  /// function parser object describing the level set
  SymFunctionPtr _func_level_set;

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

  /**
   * Evaluate the level set function at a given point.
   * @param point The point at which the level set function is to be evaluated
   * @return the value of the level set function at the given point
   */
  Real levelSetEvaluator(const Point & point);

  /**
   * Correct the position of a node based on the level set function.
   * @param node The node to be corrected
   */
  void levelSetCorrection(Node & node);

  /**
   * Generate a 2D mesh using Delaunay triangulation based on the input 2D boundary mesh and the 2D
   * hole meshes
   * @param mesh_2d The input 2D boundary mesh
   * @param hole_meshes_2d The 2D hole meshes
   * @return a unique pointer to the generated 2D mesh
   */
  std::unique_ptr<MeshBase>
  General2DDelaunay(std::unique_ptr<MeshBase> & mesh_2d,
                    std::vector<std::unique_ptr<MeshBase>> & hole_meshes_2d);
};
