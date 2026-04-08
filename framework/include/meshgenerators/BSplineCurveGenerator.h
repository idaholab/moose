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
  libMesh::Point startPoint();
  /// Return the ending point of the spline
  libMesh::Point endPoint();

  /// Subdomain ID for the elements created
  const SubdomainID _new_subdomain_id;
  /// degree of interpolating spline
  const unsigned int _degree;
  /// start point of spline
  // const libMesh::Point _start_point;
  // /// end point of spline
  // const libMesh::Point _end_point;
  /// direction of curve at start point
  const libMesh::RealVectorValue _start_dir;
  /// direction of curve at end point
  const libMesh::RealVectorValue _end_dir;
  /// sharpness of curve (measure of how close it is to the curve with three orthogonal segments)
  const libMesh::Real _sharpness;
  /// number of control points to be generated
  const unsigned int _num_cps;
  /// order of the EDGE elements to be generated
  const unsigned int _order;
  /// number of edge elements on the curve
  const unsigned int _num_elements;

  // Alternative geometry input: use boundary centroids
  /// If 'start_mesh' parameter is set, mesh providing the starting boundary, the centroid of which can be the starting point of the spline
  std::unique_ptr<MeshBase> & _start_mesh;
  /// If 'end_mesh' parameter is set, mesh providing the ending boundary, the centroid of which can be the ending point of the spline
  std::unique_ptr<MeshBase> & _end_mesh;
};
