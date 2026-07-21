//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "libmesh/mesh.h"
#include "KDTree.h"
#include "SBMBndElementBase.h"

class SBMSurfaceMeshBuilder : public GeneralUserObject
{
public:
  static InputParameters validParams();
  SBMSurfaceMeshBuilder(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  /// Returns a mutable reference because KDTree::neighborSearch is non-const (nanoflann's
  /// knnSearch threads internal scratch state through the call). The operation is logically
  /// read-only and knnSearch is re-entrant, so the same tree can be safely shared across
  /// threads without a const_cast at the call site.
  /// Callers must check hasKDTree() first; this is asserted in debug builds.
  KDTree & getKDTree() const;

  /// Whether this builder has a KDTree available. Consumers that require the tree should
  /// check this in initialSetup and mooseError with a friendly message naming both objects.
  bool hasKDTree() const { return _kd_tree != nullptr; }

  /// Get the SBM boundary elements
  const std::vector<std::unique_ptr<SBMBndElementBase>> & getBoundaryElements() const;

  /// Get the centroids of the boundary elements
  const std::vector<Point> & getCentroids() const;

  /**
   * @brief Checks whether the boundary mesh is "closed" -- i.e. has no open
   *        edges or faces.
   *
   * The implementation walks every side of every boundary element and returns
   * false as soon as it finds a side with no neighbor. On a manifold surface
   * mesh this is equivalent to watertightness; non-manifold input
   * (e.g. T-junctions, three faces sharing an edge) is not validated here
   * because the SBM workflow expects boundary meshes extracted from manifold
   * sidesets.
   *
   * @return true if every side has a neighbor; false otherwise.
   */
  bool checkWatertightness() const;

protected:
  /// Holds the mesh to ensure boundary element pointers remain valid.
  /// Avoid extracting raw MeshBase* from a function returning std::unique_ptr<MeshBase>,
  /// as it can lead to dangling pointers. Use std::unique_ptr to manage ownership safely.
  std::unique_ptr<MeshBase> _mesh;

  /// The KDTree is constructed using the centroids of the elements in the boundary mesh.
  std::unique_ptr<KDTree> _kd_tree;

  /// The boundary elements are stored in a vector of unique pointers to SBMBndElementBase.
  std::vector<std::unique_ptr<SBMBndElementBase>> _boundary_elements;

  /// The centroids of the elements in the boundary mesh are stored in a vector of Points. The sequence is the same as in _boundary_elements.
  std::vector<Point> _centroids;

  /// Configures KDTree leaf node size for performance tuning.
  const int _leaf_max_size;

  /// The name of a mesh saved via MeshGenerator `save_mesh_as` parameter.
  const std::string _bnd_mesh_name;

  /// The flag to check the watertightness of the mesh.
  const bool _check_watertightness;

  /// whether we want to build a kd-tree or not
  const bool _build_kd_tree;

private:
  /// The dimension of the embedding mesh (2D or 3D).
  const unsigned int _dim_embedding_mesh;
};
