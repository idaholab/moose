//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryMeshBuilder.h"
#include "KDTree.h"

/**
 * BoundaryMeshBuilder specialization that additionally builds a centroid KD-tree
 * over the whole-mesh SurfaceElementSet, used by UnsignedDistanceToSurfaceMesh
 * for nearest-boundary-element lookups.
 */
class SBMSurfaceMeshBuilder : public BoundaryMeshBuilder
{
public:
  static InputParameters validParams();
  SBMSurfaceMeshBuilder(const InputParameters & parameters);

  virtual void initialSetup() override;

  /// Returns a mutable reference because KDTree::neighborSearch is non-const (nanoflann's
  /// knnSearch threads internal scratch state through the call). The operation is logically
  /// read-only and knnSearch is re-entrant, so the same tree can be safely shared across
  /// threads without a const_cast at the call site.
  /// Callers must check hasKDTree() first; this is asserted in debug builds.
  KDTree & getKDTree() const;

  /// Whether this builder has a KDTree available. Consumers that require the tree should
  /// check this in initialSetup and mooseError with a friendly message naming both objects.
  bool hasKDTree() const { return _kd_tree != nullptr; }

protected:
  /// The KDTree is constructed using the centroids of the elements in the boundary mesh.
  std::unique_ptr<KDTree> _kd_tree;

  /// Configures KDTree leaf node size for performance tuning.
  const int _leaf_max_size;

  /// Whether to build the centroid KD-tree.
  const bool _build_kd_tree;
};
