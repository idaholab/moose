//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMSurfaceMeshBuilder.h"
#include "InputParameters.h"

// Register object
registerMooseObject("ShiftedBoundaryMethodApp", SBMSurfaceMeshBuilder);

InputParameters
SBMSurfaceMeshBuilder::validParams()
{
  InputParameters params = BoundaryMeshBuilder::validParams();
  params.addClassDescription(
      "Constructs boundary elements and a centroid KDTree from a pre-existing surface mesh. "
      "The surface mesh is specified in the Mesh block via `save_mesh_as`.");

  /// Add a parameter for leaf_max_size for nanoflann
  params.addParam<int>(
      "leaf_max_size",
      10,
      "Maximum number of points allowed in a leaf node of the KDTree. "
      "Smaller values yield deeper trees with faster queries but slower build times; "
      "larger values result in shallower trees with faster builds but slower queries. "
      "Benchmarking is recommended to find an optimal tradeoff for your use case.");

  /// Add a parameter to control whether to build a kd-tree or not
  params.addParam<bool>(
      "build_kd_tree",
      true,
      "Whether to build a kd-tree or not. If false, the kd-tree will not be built, "
      "and the mesh will be used directly for queries.");

  return params;
}

SBMSurfaceMeshBuilder::SBMSurfaceMeshBuilder(const InputParameters & parameters)
  : BoundaryMeshBuilder(parameters),
    _leaf_max_size(getParam<int>("leaf_max_size")),
    _build_kd_tree(getParam<bool>("build_kd_tree"))
{
}

void
SBMSurfaceMeshBuilder::initialSetup()
{
  // Build the mesh + whole-mesh SurfaceElementSet (and run shared validation).
  BoundaryMeshBuilder::initialSetup();

  // The centroid KD-tree is aligned index-for-index with surfaceElementSet().elements().
  if (_build_kd_tree)
    _kd_tree = std::make_unique<KDTree>(surfaceElementSet().centroids(), _leaf_max_size);
}

KDTree &
SBMSurfaceMeshBuilder::getKDTree() const
{
  mooseAssert(_kd_tree, "KDTree not built; callers must guard with hasKDTree() first.");
  return *_kd_tree;
}
