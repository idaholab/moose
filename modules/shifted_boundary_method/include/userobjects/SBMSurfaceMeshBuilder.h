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
#include "SBMBndEdge2.h"
#include "SBMBndTri3.h"
#include "SBMUtils.h"

class SBMSurfaceMeshBuilder : public GeneralUserObject
{
public:
  static InputParameters validParams();
  SBMSurfaceMeshBuilder(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  const KDTree & getKDTree() const { return *_kd_tree; }

  /// Get the SBM boundary elements
  const std::vector<std::unique_ptr<SBMBndElementBase>> & getBoundaryElements() const;

  /// Get the centroids of the boundary elements
  const std::vector<Point> & getCentroids() const;

  /**
   * @brief Checks whether the boundary mesh is watertight.
   *
   * For a 3D surface mesh, watertightness means that every edge (defined by two nodes)
   * is shared exactly twice across all surface elements (e.g., triangles or quadrilaterals).
   *
   * For a 2D curve mesh, watertightness means that every edge appears exactly once,
   * and every node has degree two (i.e., connected to exactly two other nodes).
   *
   * @return true if the mesh is watertight; false otherwise.
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
