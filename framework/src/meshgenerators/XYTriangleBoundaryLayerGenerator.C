//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XYTriangleBoundaryLayerGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", XYTriangleBoundaryLayerGenerator);

InputParameters
XYTriangleBoundaryLayerGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "input", "The input mesh based on which to create the conformal boundary layer.");
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
      "In which direction the boundary layer is created with respect to the side normal of the "
      "elements along the boundary of the input mesh.");

  params.addParam<std::vector<BoundaryName>>(
      "boundary_names",
      std::vector<BoundaryName>(),
      "The names of the boundaries around which the boundary layer will be created.");

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

XYTriangleBoundaryLayerGenerator::XYTriangleBoundaryLayerGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
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
}

std::unique_ptr<MeshBase>
XYTriangleBoundaryLayerGenerator::generate()
{
  const bool outward = (_boundary_layer_direction == BoundaryLayerDirection::OUTWARD);

  // The MeshGenerator system requires that the input mesh be moved out of its unique_ptr during
  // generate(). Move it now; when keep_input is false the local unique_ptr is simply dropped at
  // the end, when keep_input is true we stitch from this local pointer.
  std::unique_ptr<MeshBase> input_mesh = std::move(_input);

  auto ring = MooseMeshUtils::buildBoundaryLayerRing(*this,
                                                     *input_mesh,
                                                     _boundary_names,
                                                     _num_layers,
                                                     _thickness,
                                                     _layer_bias,
                                                     outward,
                                                     _tri_elem_type,
                                                     _output_subdomain_id,
                                                     _subdomain_name);

  // The ring carries boundary ids 0 ... 2*num_layers - 1; the innermost is bcid 1 and the
  // outermost is bcid (num_layers - 1) * 2. The "interface" side (touching the input mesh) is the
  // innermost for outward layers and the outermost for inward layers; the "surface" side (away
  // from the input mesh) is the opposite.
  const boundary_id_type ring_innermost = 1;
  const boundary_id_type ring_outermost = boundary_id_type((_num_layers - 1) * 2);
  const boundary_id_type interface_bid = outward ? ring_innermost : ring_outermost;
  const boundary_id_type surface_bid = outward ? ring_outermost : ring_innermost;

  if (_keep_input)
  {
    auto & ring_u = dynamic_cast<libMesh::UnstructuredMesh &>(*ring);
    auto & inp_u = dynamic_cast<libMesh::UnstructuredMesh &>(*input_mesh);
    if (!ring_u.is_prepared())
      ring_u.prepare_for_use();
    if (!inp_u.is_prepared())
      inp_u.prepare_for_use();
    // Avoid bcid overlap between ring and input; renumber input's bcids out of the ring's range.
    const auto ring_bids = ring_u.get_boundary_info().get_global_boundary_ids();
    const auto inp_bids = inp_u.get_boundary_info().get_global_boundary_ids();
    BoundaryID ext_id = 1;
    bool overlap = false;
    for (auto b : inp_bids)
      if (ring_bids.count(b))
        overlap = true;
    if (overlap)
    {
      const auto max_bid = std::max(ring_bids.empty() ? boundary_id_type(0) : *ring_bids.rbegin(),
                                    inp_bids.empty() ? boundary_id_type(0) : *inp_bids.rbegin());
      BoundaryID idx = 1;
      for (auto b : inp_bids)
        inp_u.get_boundary_info().renumber_id(b, max_bid + (idx++));
      ext_id = max_bid + idx;
    }
    else
      ext_id = MooseMeshUtils::getNextFreeBoundaryID(inp_u);
    inp_u.comm().max(ext_id);
    bool has_ext = false;
    MooseMeshUtils::addExternalBoundary(inp_u, ext_id, has_ext);
    if (has_ext)
    {
      libMesh::MeshSerializer s1(ring_u), s2(inp_u);
      // Stitch at the side of the ring that physically touches the input boundary. Keep the
      // interface_bid tag so the interface boundary survives for downstream naming.
      ring_u.stitch_meshes(inp_u,
                           interface_bid,
                           ext_id,
                           TOLERANCE,
                           /*clear_stitched_bcids=*/false,
                           /*verbose=*/false,
                           /*use_binary_search=*/true,
                           /*enforce_all_nodes_match_on_boundaries=*/false,
                           /*merge_boundary_nodes_all_or_nothing=*/false,
                           /*remap_subdomain_ids=*/false);
    }
  }

  auto & bi = ring->get_boundary_info();
  // Delete the interface/surface if not requested to retain
  if (_interface_name.empty())
    bi.remove_id(interface_bid);
  if (_surface_name.empty())
    bi.remove_id(surface_bid);

  // Apply interface_name / surface_name. The interface and surface bcids may collide with the
  // user-requested ids; do a two-stage shift to avoid clobbering.
  bool has_conflict = false;
  if (!_interface_name.empty())
  {
    const auto interface_user_bcid =
        MooseMeshUtils::getBoundaryIDs(*ring, {_interface_name}, true).front();
    if (interface_user_bcid == surface_bid && !_surface_name.empty())
    {
      libMesh::MeshTools::Modification::change_boundary_id(*ring, surface_bid, 10001);
      has_conflict = true;
    }
    libMesh::MeshTools::Modification::change_boundary_id(*ring, interface_bid, interface_user_bcid);
    bi.sideset_name(interface_user_bcid) = _interface_name;
  }
  if (!_surface_name.empty())
  {
    const auto surface_user_bcid =
        MooseMeshUtils::getBoundaryIDs(*ring, {_surface_name}, true).front();
    libMesh::MeshTools::Modification::change_boundary_id(
        *ring, has_conflict ? boundary_id_type(10001) : surface_bid, surface_user_bcid);
    bi.sideset_name(surface_user_bcid) = _surface_name;
  }

  // Match the parent MooseMesh's remote-element-removal setting so SetupMeshAction's sync check
  // passes when the user uses the mesh in contexts (e.g. boundary postprocessors) that flip this
  // flag on MooseMesh before the mesh is set.
  ring->allow_remote_element_removal(_mesh->allowRemoteElementRemoval());
  ring->set_isnt_prepared();
  return ring;
}
