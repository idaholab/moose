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
#include "libMeshReducedNamespace.h"

/**
 * Mesh generator to create a 1D B-spline curve mesh in 3D space
 */
class BSplineCurveGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  BSplineCurveGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Return the starting point of the spline
  Point startPoint(MeshBase & start_mesh) const;
  /// Return the ending point of the spline
  Point endPoint(MeshBase & end_mesh) const;
  /// Return the starting direction of the spline
  RealVectorValue startDirection(MeshBase & start_mesh) const;
  /// Return the ending direction of the spline
  RealVectorValue endDirection(MeshBase & end_mesh) const;

  /// Subdomain ID for the elements created
  const SubdomainID _new_subdomain_id;
  /// degree of interpolating spline
  const unsigned int _degree;
  /// sharpness of curve (measure of how close it is to the curve with three orthogonal segments)
  const Real _sharpness;
  /// number of control points to be generated
  const unsigned int _num_cps;
  /// order of the EDGE elements to be generated
  const unsigned int _order;
  /// number of edge elements on the curve
  const unsigned int _num_elements;
  /// vector of the names of the boundaries at the ends of the spline curve
  std::vector<BoundaryName> _node_set_boundaries;

  // Alternative geometry input: use boundary centroids (and boundary average normals)
  /// If 'start_mesh' parameter is set, reference to input mesh providing the starting boundary
  std::unique_ptr<MeshBase> & _start_mesh_input;
  /// If 'end_mesh' parameter is set, reference to input mesh providing the ending boundary
  std::unique_ptr<MeshBase> & _end_mesh_input;
};
