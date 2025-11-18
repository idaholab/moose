//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

// Forward declarations
class SBMSurfaceMeshBuilder;
class KDTree;
class SBMBndElementBase;

/**
 * Computes the unsigned distance to a surface mesh using KDTree nearest
 * neighbor lookup from SBMSurfaceMeshBuilder. The gradient returns the
 * unit vector pointing from the boundary element to the query point.
 */
class UnsignedDistanceToSurfaceMesh : public Function
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  UnsignedDistanceToSurfaceMesh(const InputParameters & parameters);

  /// find closest boundary element using KDTree
  SBMBndElementBase & closestBoundaryElem(const Point & p) const;

  /// find closest boundary element and return its distance vector
  RealVectorValue distanceVectorToSurface(const Point & p) const;

  /// Return unsigned distance value
  virtual Real value(Real t, const Point & p) const override;

  /// Return unit direction of distance vector (derivative)
  virtual RealGradient gradient(Real t, const Point & p) const override;

  /// Return surface normal vector on boundary
  RealVectorValue surfaceNormal(const Point & p) const;

protected:
  /// Cached pointers for speed
  KDTree * _kd_tree;
  const std::vector<dof_id_type> * _elem_id_map;
  const std::vector<std::unique_ptr<SBMBndElementBase>> * _boundary_elements;
};
