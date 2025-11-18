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
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Constructs a KDTree and boundary elements from a pre-existing surface mesh. "
      "The surface mesh is specified in the Mesh block.");

  params.addRequiredParam<std::string>(
      "surface_mesh",
      "The name of the surface mesh saved via the MeshGenerator's `save_mesh_as` parameter.");

  /// Add a parameter for leaf_max_size for nanoflann
  params.addParam<int>(
      "leaf_max_size",
      10,
      "Maximum number of points allowed in a leaf node of the KDTree. "
      "Smaller values yield deeper trees with faster queries but slower build times; "
      "larger values result in shallower trees with faster builds but slower queries. "
      "Benchmarking is recommended to find an optimal tradeoff for your use case.");

  /// Checking watertightness is not mandatory, but it is a good practice to ensure the mesh is valid for In-Out test.
  params.addParam<bool>(
      "check_watertightness",
      false,
      "Check if the mesh is watertight. If false, the mesh may not be suitable for In-Out tests.");

  /// Add a parameter to control whether to build a kd-tree or not
  params.addParam<bool>(
      "build_kd_tree",
      true,
      "Whether to build a kd-tree or not. If false, the kd-tree will not be built, "
      "and the mesh will be used directly for queries.");

  return params;
}

SBMSurfaceMeshBuilder::SBMSurfaceMeshBuilder(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _leaf_max_size(getParam<int>("leaf_max_size")),
    _bnd_mesh_name(getParam<std::string>("surface_mesh")),
    _check_watertightness(getParam<bool>("check_watertightness")),
    _build_kd_tree(getParam<bool>("build_kd_tree")),
    _dim_embedding_mesh(_fe_problem.mesh().dimension() /*MooseMesh*/)
{
}

void
SBMSurfaceMeshBuilder::initialSetup()
{
  auto & mesh_generator_system = _app.getMeshGeneratorSystem();

  _mesh = mesh_generator_system.getSavedMesh(_bnd_mesh_name);

  const auto expected_dim_embedding_mesh = _mesh->mesh_dimension() + 1;

  if (!_mesh->is_replicated())
    mooseError(
        "The mesh is distributed. Please use a serial mesh for SBMSurfaceMeshBuilder, which is "
        "important for our In-Out test later.");

  if (_dim_embedding_mesh != expected_dim_embedding_mesh)
    mooseError("The original mesh dimension (",
               _dim_embedding_mesh,
               ") does not match the expected mesh dimension (",
               expected_dim_embedding_mesh,
               ").");

  const std::size_t num_elems = _mesh->n_elem();

  _centroids.resize(num_elems);
  _elem_id_map.resize(num_elems);
  _boundary_elements.resize(num_elems);

  int i = 0;
  for (auto elem_it = _mesh->active_elements_begin(); elem_it != _mesh->active_elements_end();
       ++elem_it)
  {
    const Elem * elem = *elem_it;

    const auto centroid = elem->vertex_average();
    if (_build_kd_tree)
      _centroids[i] = centroid;

    _elem_id_map[i] = elem->id();

    if (elem->type() == EDGE2)
      _boundary_elements[i] = std::make_unique<SBMBndEdge2>(elem);
    else if (elem->type() == TRI3)
      _boundary_elements[i] = std::make_unique<SBMBndTri3>(elem);
    else
      mooseError("Unsupported element type in SBMSurfaceMeshBuilder::initialSetup()");

    ++i;
  }

  if (_build_kd_tree)
    _kd_tree = std::make_unique<KDTree>(_centroids, _leaf_max_size);

  if (_check_watertightness)
  {
    bool watertight = checkWatertightness();
    if (!watertight)
      mooseInfo("The mesh is not watertight. It may not be suitable for In-Out tests.");
    else
      mooseInfo("The mesh is watertight. It is suitable for In-Out tests.");
  }
}

bool
SBMSurfaceMeshBuilder::checkWatertightness() const
{
  return SBMUtils::checkWatertightnessFromRawElems(std::vector<const Elem *>(
      _mesh->active_element_ptr_range().begin(), _mesh->active_element_ptr_range().end()));
}
