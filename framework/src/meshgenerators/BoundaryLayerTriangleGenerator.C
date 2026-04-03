//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryLayerTriangleGenerator.h"

#include "MooseMeshUtils.h"
#include "MooseUtils.h"
#include "GeometryUtils.h"

#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/mesh_modification.h"

registerMooseObject("MooseApp", BoundaryLayerTriangleGenerator);

InputParameters
BoundaryLayerTriangleGenerator::validParams()
{
  InputParameters params = LayerDelaunayBase::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "input", "The input mesh based on which to create the conformal boundary layer layers.");

  params.addRequiredParam<Real>("thickness",
                                "The total thickness of the boundary layer to be created.");

  params.addRangeCheckedParam<unsigned int>(
      "num_layers", 1, "num_layers>0", "The total number of boundary layers to be created.");

  params.addParam<Real>(
      "layer_bias",
      1.0,
      "The bias factor for the thickness of each layer. A value > 1 leads to thicker layers away "
      "from the input mesh, while a value < 1 leads to thicker layers close to the input mesh. The "
      "default value of 1 leads to uniform layer thickness.");

  MooseEnum boundary_layer_direction("OUTWARD INWARD", "OUTWARD");

  params.addParam<MooseEnum>(
      "boundary_layer_direction",
      boundary_layer_direction,
      "In which direction the boundary layer is created with respect to the side "
      "normal of the elements along the boundary of the input mesh.");

  params.addParam<std::vector<BoundaryName>>(
      "boundary_names",
      std::vector<BoundaryName>(),
      "The names of the boundaries around which the coating will be created.");

  params.addParam<BoundaryName>("interface_name",
                                "The optional boundary name to be assigned to the interface "
                                "between the generated boundary layer and the input mesh.");
  params.addParam<BoundaryName>(
      "surface_name",
      "The optional boundary name to be assigned to the surface of the generated boundary layer.");

  params.addParam<SubdomainName>("subdomain_name",
                                 "Subdomain name to set for the boundary layer mesh.");
  params.addParam<SubdomainID>("subdomain_id", "Subdomain id to set for the boundary layer mesh.");

  MooseEnum tri_elem_type("TRI3 TRI6 TRI7", "TRI3");

  params.addParam<MooseEnum>("tri_elem_type",
                             tri_elem_type,
                             "The type of triangular elements to use for the boundary layer.");

  params.addParam<bool>("keep_input",
                        false,
                        "Whether to keep the input mesh in the final output. If false, only the "
                        "boundary layers will be included in the output mesh.");

  params.addClassDescription(
      "Generate a 2D layered mesh that represents a conformal boundary layer along "
      "the boundary of an input 2D mesh or a 1D loop mesh.");

  return params;
}

BoundaryLayerTriangleGenerator::BoundaryLayerTriangleGenerator(const InputParameters & parameters)
  : LayerDelaunayBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _thickness(getParam<Real>("thickness")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _layer_bias(getParam<Real>("layer_bias")),
    _boundary_layer_direction(
        getParam<MooseEnum>("boundary_layer_direction").template getEnum<BoundaryLayerDirection>()),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary_names")),
    _interface_name(isParamValid("interface_name") ? getParam<BoundaryName>("interface_name")
                                                   : BoundaryName()),
    _surface_name(isParamValid("surface_name") ? getParam<BoundaryName>("surface_name")
                                               : BoundaryName()),
    _subdomain_name(isParamValid("subdomain_name") ? getParam<SubdomainName>("subdomain_name")
                                                   : SubdomainName()),
    _output_subdomain_id(isParamValid("subdomain_id") ? getParam<SubdomainID>("subdomain_id") : 0),
    _tri_elem_type(parameters.get<MooseEnum>("tri_elem_type")),
    _keep_input(getParam<bool>("keep_input"))
{
  declareMeshForSub("input");

  _build_mesh = &getMeshByName(create_conformal_boundary_layer_mesh(
      _num_layers,
      _thickness,
      _layer_bias,
      _boundary_layer_direction == BoundaryLayerDirection::OUTWARD,
      _keep_input,
      _input_name,
      _boundary_names,
      _tri_elem_type,
      _output_subdomain_id,
      _subdomain_name,
      _interface_name.empty() ? libMesh::BoundaryInfo::invalid_id : 0,
      _surface_name.empty() ? libMesh::BoundaryInfo::invalid_id : 1));
}

std::unique_ptr<MeshBase>
BoundaryLayerTriangleGenerator::generate()
{
  bool has_conflict = false;
  if (_interface_name.size())
  {
    const auto interface_bcid =
        MooseMeshUtils::getBoundaryIDs(**_build_mesh, {_interface_name}, true).front();

    // shift the other id if there is a conflict with the default
    if (interface_bcid == 1)
    {
      libMesh::MeshTools::Modification::change_boundary_id(**_build_mesh, 1, 2);
      has_conflict = true;
    }
    libMesh::MeshTools::Modification::change_boundary_id(**_build_mesh, 0, interface_bcid);

    (*_build_mesh)->get_boundary_info().sideset_name(interface_bcid) = _interface_name;
  }
  if (_surface_name.size())
  {
    const auto surface_bcid =
        MooseMeshUtils::getBoundaryIDs(**_build_mesh, {_surface_name}, true).front();
    libMesh::MeshTools::Modification::change_boundary_id(
        **_build_mesh, has_conflict ? 2 : 1, surface_bcid);
    (*_build_mesh)->get_boundary_info().sideset_name(surface_bcid) = _surface_name;
  }

  return std::move(*_build_mesh);
}
