//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "STLSubdomainGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "STLManifold.h"

#include "libmesh/elem.h"

#include <set>

registerMooseObject("MooseApp", STLSubdomainGenerator);

InputParameters
STLSubdomainGenerator::validParams()
{
  // This interface intentionally mirrors other point-sampling subdomain tagging generators.
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify.");
  params.addRequiredParam<FileName>("stl_file", "The STL file defining a closed manifold.");
  params.addRequiredParam<subdomain_id_type>("block_id", "Subdomain id to assign.");
  params.addParam<SubdomainName>("block_name", "Optional subdomain name to assign.");
  params.addParam<MooseEnum>(
      "location", location, "Control whether the manifold interior or exterior is tagged.");
  params.addParam<std::vector<SubdomainName>>("restricted_subdomains",
                                              "Only reset subdomain ID for given subdomains.");
  params.addParam<RealVectorValue>(
      "scale",
      RealVectorValue(1, 1, 1),
      "Per-axis scaling applied to the STL geometry before rotation and translation.");
  params.addParam<RealVectorValue>(
      "rotation",
      RealVectorValue(0, 0, 0),
      "Extrinsic Euler rotation angles in degrees applied to the STL geometry after scaling.");
  params.addParam<RealVectorValue>("translation",
                                   RealVectorValue(0, 0, 0),
                                   "Translation applied after STL scaling and rotation.");
  params.addRangeCheckedParam<Real>(
      "surface_tolerance",
      1e-10,
      "surface_tolerance>0",
      "Absolute geometric tolerance used for manifold validation and near-surface classification. "
      "Choose this relative to the STL length scale and expected coordinate noise.");
  params.addClassDescription(
      "Changes the subdomain ID of elements whose vertex-average point lies inside or outside a "
      "closed STL manifold.");
  return params;
}

STLSubdomainGenerator::STLSubdomainGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<subdomain_id_type>("block_id")),
    _has_restriction(isParamValid("restricted_subdomains")),
    _stl_file(getParam<FileName>("stl_file")),
    _scale(getParam<RealVectorValue>("scale")),
    _rotation(getParam<RealVectorValue>("rotation")),
    _translation(getParam<RealVectorValue>("translation")),
    _surface_tolerance(getParam<Real>("surface_tolerance"))
{
  for (unsigned int i = 0; i < Moose::dim; ++i)
    if (_scale(i) <= 0.0)
      paramError("scale", "all components must be strictly positive.");
}

std::unique_ptr<MeshBase>
STLSubdomainGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // The STL manifold describes a volume boundary, so the target mesh must also be volumetric.
  if (mesh->mesh_dimension() != 3)
    paramError("input", "Only 3D meshes are supported.");

  std::set<SubdomainID> restricted_ids;
  if (_has_restriction)
  {
    // Resolve user block names to ids once before the element loop.
    const auto & names = getParam<std::vector<SubdomainName>>("restricted_subdomains");
    for (const auto & name : names)
    {
      if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
        paramError("restricted_subdomains", "The block '", name, "' was not found in the mesh");

      restricted_ids.insert(MooseMeshUtils::getSubdomainID(name, *mesh));
    }
  }

  if (MooseMeshUtils::hasSubdomainID(*mesh, _block_id) && mesh->processor_id() == 0)
    mooseInfo("The requested block_id ",
              _block_id,
              " already exists on the input mesh. Elements outside the STL manifold that are "
              "already assigned to this block will remain unchanged.");

  // Build the classifier up front so the per-element loop only performs point-in-manifold queries.
  STLManifold manifold(_stl_file, _scale, _rotation, _translation, _surface_tolerance);

  // On distributed meshes, only locally owned active elements are modified on each rank.
  for (const auto & elem : mesh->active_local_element_ptr_range())
  {
    if (_has_restriction && restricted_ids.count(elem->subdomain_id()) == 0)
      continue;

    // This generator intentionally samples at the vertex average instead of the true geometric
    // centroid, matching the inexpensive behavior of other subdomain tagging generators.
    const bool contains = manifold.contains(elem->vertex_average());
    if ((contains && _location == "INSIDE") || (!contains && _location == "OUTSIDE"))
      elem->subdomain_id() = _block_id;
  }

  // Preserve the optional user-facing subdomain name for the assigned id.
  if (isParamValid("block_name"))
    mesh->subdomain_name(_block_id) = getParam<SubdomainName>("block_name");

  // Mark derived mesh data as stale because element subdomain assignments have changed.
  mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
