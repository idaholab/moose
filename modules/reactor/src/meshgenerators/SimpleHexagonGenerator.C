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
  MooseEnum element_type("TRI QUAD HYBRID", "TRI");
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
  params.addRangeCheckedParam<unsigned int>(
      "radial_intervals",
      "radial_intervals>1",
      "Number of pin radial meshing rings (only applicable when 'element_type' is 'HYBRID').");
  params.addRangeCheckedParam<std::vector<subdomain_id_type>>(
      "block_id",
      "block_id>=0",
      "Optional customized block id; two ids are needed for HYBRID 'element_type'.");
  params.addParam<std::vector<SubdomainName>>(
      "block_name",
      "Optional customized block name; two names are needed for HYBRID 'element_type'.");
  params.addRangeCheckedParam<boundary_id_type>("external_boundary_id",
                                                "external_boundary_id>=0",
                                                "Optional customized external boundary id.");
  params.addParam<BoundaryName>("external_boundary_name",
                                "Optional customized external boundary name.");
  params.addParamNamesToGroup("block_id block_name external_boundary_id external_boundary_name",
                              "Customized Subdomain/Boundary");
  params.addClassDescription(
      "This SimpleHexagonGenerator object is designed to generate a simple hexagonal mesh that "
      "only contains six simple azimuthal triangular elements, two quadrilateral elements, or six "
      "central azimuthal triangular elements plus a several layers of quadrilateral elements.");

  return params;
}

SimpleHexagonGenerator::SimpleHexagonGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _element_type(getParam<MooseEnum>("element_type").template getEnum<ElemType>()),
    _hexagon_size(getParam<Real>("hexagon_size")),
    _hexagon_size_style(
        getParam<MooseEnum>("hexagon_size_style").template getEnum<PolygonSizeStyle>()),
    _radial_intervals(isParamValid("radial_intervals")
                          ? getParam<unsigned int>("radial_intervals")
                          : (_element_type == ElemType::HYBRID ? 2 : 1)),
    _block_id(isParamValid("block_id") ? getParam<std::vector<subdomain_id_type>>("block_id")
                                       : std::vector<subdomain_id_type>()),
    _block_name(isParamValid("block_name") ? getParam<std::vector<SubdomainName>>("block_name")
                                           : std::vector<SubdomainName>()),
    _boundary_id_valid(isParamValid("external_boundary_id")),
    _external_boundary_id(isParamValid("external_boundary_id")
                              ? getParam<boundary_id_type>("external_boundary_id")
                              : 0),
    _external_boundary_name(isParamValid("external_boundary_name")
                                ? getParam<BoundaryName>("external_boundary_name")
                                : BoundaryName())
{
  if (_radial_intervals > 1 && _element_type != ElemType::HYBRID)
    paramError(
        "radial_intervals",
        "A non-unity 'radial_intervals' value is only supported when 'element_type' is 'HYBRID'.");
  _pitch = 2.0 * (_hexagon_size_style == PolygonSizeStyle::apothem
                      ? _hexagon_size
                      : _hexagon_size * std::cos(M_PI / (Real)HEXAGON_NUM_SIDES));
  if ((_block_id.size() > 1) && _element_type != ElemType::HYBRID)
    paramError(
        "block_id",
        "if provided, the size of this parameter must be one if 'element_type' is TRI or QUAD.");
  if ((_block_id.size() != 0 && _block_id.size() != 2) && _element_type == ElemType::HYBRID)
    paramError("block_id",
               "if provided, the size of this parameter must be two if 'element_type' is HYBRID.");
  if (_block_name.size() != 0 && _block_name.size() != _block_id.size())
    paramError("block_name", "if provided, this parameter must have the same size as 'block_id'.");
  declareMeshProperty<unsigned int>("background_intervals_meta", _radial_intervals);
  declareMeshProperty<dof_id_type>("node_id_background_meta",
                                   _radial_intervals * HEXAGON_NUM_SIDES);
  declareMeshProperty<Real>("max_radius_meta", 0.0);
  declareMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", {1, 1, 1, 1, 1, 1});
  declareMeshProperty<Real>("pitch_meta", _pitch);
}

std::unique_ptr<MeshBase>
SimpleHexagonGenerator::generate()
{
  const Real radius = _pitch / std::sqrt(3.0);
  auto mesh = buildReplicatedMesh(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  // In the ElemType::TRI mode, total nodes number is 6 * _radial_intervals + 1, while
  // _radial_intervals must be trivial (1);
  // In the ElemType::QUAD model, total nodes number is 6 * _radial_intervals, while
  // _radial_intervals must be trivial (1);
  // In the ElemType::HYBRID mode, total nodes number is 6 * _radial_intervals + 1.
  std::vector<Node *> nodes(HEXAGON_NUM_SIDES * _radial_intervals +
                            (_element_type != ElemType::QUAD));
  if (_element_type != ElemType::QUAD)
  {
    // The trivial center node with node_id = 6 (HEXAGON_NUM_SIDES) * _radial_intervals
    Point center_p = Point(0.0, 0.0, 0.0);
    nodes[HEXAGON_NUM_SIDES * _radial_intervals] =
        mesh->add_point(center_p, HEXAGON_NUM_SIDES * _radial_intervals);
  }
  // loop to create nodes by radial layers
  for (unsigned int j = 0; j < _radial_intervals; j++)
    // loop to create the six nodes in each layer
    for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
    {
      Point side_p = Point(radius * (Real)(_radial_intervals - j) / (Real)_radial_intervals *
                               sin(M_PI / ((Real)HEXAGON_NUM_SIDES / 2.0) * (Real)i),
                           radius * (Real)(_radial_intervals - j) / (Real)_radial_intervals *
                               cos(M_PI / ((Real)HEXAGON_NUM_SIDES / 2.0) * (Real)i),
                           0.0);
      nodes[j * HEXAGON_NUM_SIDES + i] = mesh->add_point(side_p, j * HEXAGON_NUM_SIDES + i);
    }

  if (_element_type != ElemType::QUAD)
  {
    // loop to create outer layer QUAD elements
    // Note that the direction is from outer to inner
    for (unsigned int j = 0; j < _radial_intervals - 1; j++)
    {
      // loop to create the six QUAD elements for each layer
      for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
      {
        Elem * elem = mesh->add_elem(new Quad4);
        elem->set_node(0) = nodes[HEXAGON_NUM_SIDES * j + i];
        elem->set_node(1) = nodes[HEXAGON_NUM_SIDES * j + (i + 1) % HEXAGON_NUM_SIDES];
        elem->set_node(2) = nodes[HEXAGON_NUM_SIDES * (j + 1) + (i + 1) % HEXAGON_NUM_SIDES];
        elem->set_node(3) = nodes[HEXAGON_NUM_SIDES * (j + 1) + i];
        // Assign the default external boundary id if applicable so that the mesh can be used as
        // input of `PatterneHexMeshGenerator`.
        if (j == 0)
          boundary_info.add_side(elem, 0, OUTER_SIDESET_ID);
        // Default subdomain id
        elem->subdomain_id() = 2;
      }
    }
    // loop to create the six TRI elements
    for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
    {
      Elem * elem = mesh->add_elem(new Tri3);
      elem->set_node(0) = nodes[HEXAGON_NUM_SIDES * _radial_intervals];
      elem->set_node(2) = nodes[HEXAGON_NUM_SIDES * (_radial_intervals - 1) + i];
      elem->set_node(1) =
          nodes[HEXAGON_NUM_SIDES * (_radial_intervals - 1) + (i + 1) % HEXAGON_NUM_SIDES];
      // Assign the default external boundary id if applicable so that the mesh can be used as input
      // of `PatterneHexMeshGenerator`.
      if (_element_type != ElemType::HYBRID)
        boundary_info.add_side(elem, 1, OUTER_SIDESET_ID);
      // Default subdomain id
      elem->subdomain_id() = 1;
    }
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
  if (!_block_id.empty())
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      if (elem->subdomain_id() == 1)
        elem->subdomain_id() = _block_id.front();
      else
        elem->subdomain_id() = _block_id.back();
    }
  if (!_block_name.empty())
    for (unsigned int i = 0; i < _block_name.size(); i++)
      mesh->subdomain_name(_block_id[i]) = _block_name[i];

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
