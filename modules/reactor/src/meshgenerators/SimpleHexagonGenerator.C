//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleHexagonGenerator.h"

// C++ includes
#include <cmath>

registerMooseObject("ReactorApp", SimpleHexagonGenerator);

InputParameters
SimpleHexagonGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  MooseEnum element_type("TRI QUAD", "TRI");
  params.addParam<MooseEnum>("element_type",
                             element_type,
                             "Whether the simple hexagon mesh is made of TRI or QUAD elements.");
  params.addRequiredRangeCheckedParam<Real>(
      "hexagon_size", "hexagon_size>0.0", "Size of the hexagon to be generated.");
  MooseEnum hexagon_size_style("apothem radius", "apothem");
  params.addParam<MooseEnum>(
      "hexagon_size_style",
      hexagon_size_style,
      "Style in which the hexagon size is given (default: apothem i.e. half-pitch). Option: " +
          hexagon_size_style.getRawNames());
  params.addRangeCheckedParam<subdomain_id_type>(
      "block_id", "block_id>=0", "Optional customized block id.");
  params.addParam<SubdomainName>("block_name", "Optional customized block name.");
  params.addRangeCheckedParam<boundary_id_type>("external_boundary_id",
                                                "external_boundary_id>=0",
                                                "Optional customized external boundary id.");
  params.addParam<std::string>("external_boundary_name",
                               "Optional customized external boundary name.");
  params.addParamNamesToGroup("block_id block_name external_boundary_id external_boundary_name",
                              "Customized Subdomain/Boundary");
  params.addClassDescription(
      "This SimpleHexagonGenerator object is designed to generate a simple hexagonal mesh that "
      "only contains six simple azimuthal triangle slices.");

  return params;
}

SimpleHexagonGenerator::SimpleHexagonGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _element_type(getParam<MooseEnum>("element_type").template getEnum<ElemType>()),
    _hexagon_size(getParam<Real>("hexagon_size")),
    _hexagon_size_style(
        getParam<MooseEnum>("hexagon_size_style").template getEnum<PolygonSizeStyle>()),
    _block_id_valid(isParamValid("block_id")),
    _block_id(isParamValid("block_id") ? getParam<subdomain_id_type>("block_id") : 0),
    _block_name(isParamValid("block_name") ? getParam<SubdomainName>("block_name")
                                           : SubdomainName()),
    _boundary_id_valid(isParamValid("external_boundary_id")),
    _external_boundary_id(isParamValid("external_boundary_id")
                              ? getParam<boundary_id_type>("external_boundary_id")
                              : 0),
    _external_boundary_name(isParamValid("external_boundary_name")
                                ? getParam<std::string>("external_boundary_name")
                                : std::string()),
    _pitch_meta(declareMeshProperty<Real>("pitch_meta", 0.0)),
    _background_intervals_meta(declareMeshProperty<unsigned int>("background_intervals_meta", 1)),
    _node_id_background_meta(declareMeshProperty<unsigned int>("node_id_background_meta", 6)),
    _max_radius_meta(declareMeshProperty<Real>("max_radius_meta", 0.0)),
    _num_sectors_per_side_meta(declareMeshProperty<std::vector<unsigned int>>(
        "num_sectors_per_side_meta", {1, 1, 1, 1, 1, 1}))
{
  _pitch = 2.0 * (_hexagon_size_style == PolygonSizeStyle::apothem
                      ? _hexagon_size
                      : _hexagon_size * std::cos(M_PI / (Real)HEXAGON_NUM_SIDES));
  _pitch_meta = _pitch;
}

std::unique_ptr<MeshBase>
SimpleHexagonGenerator::generate()
{
  const Real radius = _pitch / std::sqrt(3.0);
  auto mesh = buildReplicatedMesh(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  // A total of 6 + 1 = 7 nodes in this simple hexagon mesh
  std::vector<Node *> nodes(HEXAGON_NUM_SIDES + (_element_type == ElemType::TRI));
  if (_element_type == ElemType::TRI)
  {
    // The trivial center node with node_id = 6 (HEXAGON_NUM_SIDES)
    Point center_p = Point(0.0, 0.0, 0.0);
    nodes[HEXAGON_NUM_SIDES] = mesh->add_point(center_p, HEXAGON_NUM_SIDES);
  }
  // loop to create the six vertex nodes (node_id 0 ~ 5)
  for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
  {
    Point side_p = Point(radius * sin(M_PI / ((Real)HEXAGON_NUM_SIDES / 2.0) * (Real)i),
                         radius * cos(M_PI / ((Real)HEXAGON_NUM_SIDES / 2.0) * (Real)i),
                         0.0);
    nodes[i] = mesh->add_point(side_p, i);
  }

  if (_element_type == ElemType::TRI)
    // loop to create the six elements
    for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
    {
      Elem * elem = mesh->add_elem(new Tri3);
      elem->set_node(0) = nodes[HEXAGON_NUM_SIDES];
      elem->set_node(2) = nodes[i];
      elem->set_node(1) = nodes[(i + 1) % HEXAGON_NUM_SIDES];
      // Assign the default external boundary id so that the mesh can be used as input of
      // `PatterneHexMeshGenerator`.
      boundary_info.add_side(elem, 1, OUTER_SIDESET_ID);
      // Default subdomain id
      elem->subdomain_id() = 1;
    }
  else
    // loop to create the two elements
    for (unsigned int i = 0; i < HEXAGON_NUM_SIDES / 3; i++)
    {
      Elem * elem = mesh->add_elem(new Quad4);
      elem->set_node(0) = nodes[i * 3];
      elem->set_node(1) = nodes[(i * 3 + 3) % HEXAGON_NUM_SIDES];
      elem->set_node(2) = nodes[i * 3 + 2];
      elem->set_node(3) = nodes[i * 3 + 1];
      // Assign the default external boundary id so that the mesh can be used as input of
      // `PatterneHexMeshGenerator`.
      boundary_info.add_side(elem, 1, OUTER_SIDESET_ID);
      boundary_info.add_side(elem, 2, OUTER_SIDESET_ID);
      boundary_info.add_side(elem, 3, OUTER_SIDESET_ID);
      // Default subdomain id
      elem->subdomain_id() = 1;
    }

  // Assign customized (optional) subdomain id/name and external boundary id/name.
  const subdomain_id_type block_id_new(_block_id_valid ? _block_id : 1);
  if (_block_id_valid)
    for (const auto & elem : mesh->active_element_ptr_range())
      if (elem->subdomain_id() == 1)
        elem->subdomain_id() = block_id_new;
  if (!_block_name.empty())
    mesh->subdomain_name(block_id_new) = _block_name;

  if (_boundary_id_valid)
    MooseMesh::changeBoundaryId(*mesh, OUTER_SIDESET_ID, _external_boundary_id, false);
  if (!_external_boundary_name.empty())
  {
    mesh->get_boundary_info().sideset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
    mesh->get_boundary_info().nodeset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
  }

  mesh->prepare_for_use();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
