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

defineLegacyParams(SimpleHexagonGenerator);

InputParameters
SimpleHexagonGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
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
  params.addClassDescription(
      "This SimpleHexagonGenerator object is designed to generate a simple hexagonal mesh that "
      "only contains six simple azimuthal triangle slices.");

  return params;
}

SimpleHexagonGenerator::SimpleHexagonGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _hexagon_size(getParam<Real>("hexagon_size")),
    _hexagon_size_style(getParam<MooseEnum>("hexagon_size_style").template getEnum<HexagonStyle>()),
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
  _pitch = 2.0 * (_hexagon_size_style == HexagonStyle::apothem
                      ? _hexagon_size
                      : _hexagon_size * std::cos(M_PI / (Real)HEXAGON_NUM_SIDES));
}

std::unique_ptr<MeshBase>
SimpleHexagonGenerator::generate()
{
  const Real radius = _pitch / std::sqrt(3.0);
  auto mesh = buildReplicatedMesh(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  std::vector<Node *> nodes(HEXAGON_NUM_SIDES + 1);
  Point center_p = Point(0.0, 0.0, 0.0);
  nodes[HEXAGON_NUM_SIDES] = mesh->add_point(center_p, HEXAGON_NUM_SIDES);
  for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
  {
    Point side_p = Point(radius * sin(M_PI / ((Real)HEXAGON_NUM_SIDES / 2.0) * (Real)i),
                         radius * cos(M_PI / ((Real)HEXAGON_NUM_SIDES / 2.0) * (Real)i),
                         0.0);
    nodes[i] = mesh->add_point(side_p, i);
  }
  for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
  {
    Elem * elem = mesh->add_elem(new Tri3);
    elem->set_node(0) = nodes[HEXAGON_NUM_SIDES];
    elem->set_node(2) = nodes[i % HEXAGON_NUM_SIDES];
    elem->set_node(1) = nodes[(i + 1) % HEXAGON_NUM_SIDES];
    boundary_info.add_side(elem, 1, OUTER_SIDESET_ID);
    elem->subdomain_id() = 1;
  }

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
    mesh->boundary_info->sideset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
    mesh->boundary_info->nodeset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
  }

  _pitch_meta = _pitch;

  return dynamic_pointer_cast<MeshBase>(mesh);
}
