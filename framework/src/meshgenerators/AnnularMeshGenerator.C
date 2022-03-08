//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnnularMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"

registerMooseObject("MooseApp", AnnularMeshGenerator);

InputParameters
AnnularMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRangeCheckedParam<unsigned int>(
      "nr", 1, "nr>0", "Number of elements in the radial direction");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "nt", "nt>0", "Number of elements in the angular direction");
  params.addRequiredRangeCheckedParam<Real>(
      "rmin",
      "rmin>=0.0",
      "Inner radius.  If rmin=0 then a disc mesh (with no central hole) will be created.");
  params.addRequiredParam<Real>("rmax", "Outer radius");
  params.addParam<std::vector<Real>>("radial_positions",
                                     "Directly prescribed positions of intermediate radial nodes");
  params.addParam<bool>("equal_area",
                        false,
                        "Whether to select the radial discretization "
                        "to achieve equal areas of each ring");
  params.addDeprecatedParam<Real>("tmin",
                                  0.0,
                                  "Minimum angle, measured in radians anticlockwise from x axis",
                                  "Use dmin instead");
  params.addDeprecatedParam<Real>(
      "tmax",
      2 * M_PI,
      "Maximum angle, measured in radians anticlockwise from x axis. If "
      "tmin=0 and tmax=2Pi an annular mesh is created. "
      "Otherwise, only a sector of an annulus is created",
      "Use dmin instead");
  params.addParam<Real>(
      "dmin", 0.0, "Minimum degree, measured in degrees anticlockwise from x axis");
  params.addParam<Real>("dmax",
                        360.0,
                        "Maximum angle, measured in degrees anticlockwise from x axis. If "
                        "dmin=0 and dmax=360 an annular mesh is created. "
                        "Otherwise, only a sector of an annulus is created");
  params.addRangeCheckedParam<Real>("growth_r",
                                    1.0,
                                    "growth_r!=0.0",
                                    "The ratio of radial sizes of successive rings of elements");
  params.addParam<SubdomainID>(
      "quad_subdomain_id", 0, "The subdomain ID given to the QUAD4 elements");
  params.addParam<SubdomainID>("tri_subdomain_id",
                               1,
                               "The subdomain ID given to the TRI3 elements "
                               "(these exist only if rmin=0, and they exist "
                               "at the center of the disc");
  params.addClassDescription("For rmin>0: creates an annular mesh of QUAD4 elements. For rmin=0: "
                             "creates a disc mesh of QUAD4 and TRI3 elements. Boundary sidesets "
                             "are created at rmax and rmin, and given these names. If dmin!=0 and "
                             "dmax!=360, a sector of an annulus or disc is created. In this case "
                             "boundary sidesets are also created at dmin and dmax, and "
                             "given these names");

  return params;
}

AnnularMeshGenerator::AnnularMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _nt(getParam<unsigned int>("nt")),
    _rmin(getParam<Real>("rmin")),
    _rmax(getParam<Real>("rmax")),
    _radial_positions(getParam<std::vector<Real>>("radial_positions")),
    _nr(parameters.isParamSetByUser("radial_positions") ? _radial_positions.size() + 1
                                                        : getParam<unsigned int>("nr")),
    _dmin(parameters.isParamSetByUser("tmin") ? getParam<Real>("tmin") / M_PI * 180.0
                                              : getParam<Real>("dmin")),
    _dmax(parameters.isParamSetByUser("tmax") ? getParam<Real>("tmax") / M_PI * 180.0
                                              : getParam<Real>("dmax")),
    _radians((parameters.isParamSetByUser("tmin") || parameters.isParamSetByUser("tmax")) ? true
                                                                                          : false),
    _growth_r(getParam<Real>("growth_r")),
    _len(_growth_r == 1.0 ? (_rmax - _rmin) / _nr
                          : (_rmax - _rmin) * (1.0 - std::abs(_growth_r)) /
                                (1.0 - std::pow(std::abs(_growth_r), _nr))),
    _full_annulus(_dmin == 0.0 && _dmax == 360),
    _quad_subdomain_id(getParam<SubdomainID>("quad_subdomain_id")),
    _tri_subdomain_id(getParam<SubdomainID>("tri_subdomain_id")),
    _equal_area(getParam<bool>("equal_area"))
{
  if ((parameters.isParamSetByUser("tmin") || parameters.isParamSetByUser("tmax")) &&
      (parameters.isParamSetByUser("dmin") || parameters.isParamSetByUser("dmax")))
    paramError("tmin",
               "You specified the angles using both degrees and radians. Please use degrees.");

  if (_radial_positions.size() != 0)
  {
    if (parameters.isParamSetByUser("nr"))
      paramError("nr", "The 'nr' parameter cannot be specified together with 'radial_positions'");
    if (parameters.isParamSetByUser("equal_area"))
      paramError("equal_area",
                 "The 'equal_area' parameter cannot be specified together with 'radial_positions'");
    if (parameters.isParamSetByUser("growth_r"))
      paramError("growth_r",
                 "The 'growth_r' parameter cannot be specified together with 'radial_positions'");
    for (auto rpos : _radial_positions)
      if (rpos <= _rmin || rpos >= _rmax)
        paramError(
            "radial_positions",
            "The following provided value is not within the bounds between 'rmin' and 'rmax': ",
            rpos);
  }

  if (_equal_area && parameters.isParamSetByUser("growth_r"))
    paramError("growth_r", "The 'growth_r' parameter cannot be combined with 'equal_area'");

  if (_rmax <= _rmin)
    paramError("rmax", "rmax must be greater than rmin");
  if (_dmax <= _dmin)
    paramError("dmax", "dmax must be greater than dmin");
  if (_dmax - _dmin > 360)
    paramError("dmax", "dmax - dmin must be <= 360");
  if (_nt <= (_dmax - _dmin) / 180.0)
    paramError("nt",
               "nt must be greater than (dmax - dmin) / 180 in order to avoid inverted "
               "elements");
  if (_quad_subdomain_id == _tri_subdomain_id)
    paramError("quad_subdomain_id", "quad_subdomain_id must not equal tri_subdomain_id");
}

std::unique_ptr<MeshBase>
AnnularMeshGenerator::generate()
{
  // Have MOOSE construct the correct libMesh::Mesh object using Mesh block and CLI parameters.
  auto mesh = buildMeshBaseObject();

  const Real dt = (_dmax - _dmin) / _nt;

  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  const unsigned num_angular_nodes = (_full_annulus ? _nt : _nt + 1);
  const unsigned num_nodes =
      (_rmin > 0.0 ? (_nr + 1) * num_angular_nodes : _nr * num_angular_nodes + 1);
  const unsigned min_nonzero_layer_num = (_rmin > 0.0 ? 0 : 1);
  std::vector<Node *> nodes(num_nodes);
  unsigned node_id = 0;

  // add nodes at rmax that aren't yet connected to any elements
  Real current_r = _rmax;
  for (unsigned angle_num = 0; angle_num < num_angular_nodes; ++angle_num)
  {
    const Real angle = _dmin + angle_num * dt;
    const Real x = current_r * std::cos(angle * M_PI / 180.0);
    const Real y = current_r * std::sin(angle * M_PI / 180.0);
    nodes[node_id] = mesh->add_point(Point(x, y, 0.0), node_id);
    node_id++;
  }

  // first value for the ring outer radius, only used for _equal_area
  Real outer_r = _rmax;

  // add nodes at smaller radii, and connect them with elements
  for (unsigned layer_num = _nr; layer_num > min_nonzero_layer_num; --layer_num)
  {
    if (layer_num == 1)
      current_r = _rmin; // account for precision loss
    else if (_radial_positions.size() > 0)
      current_r = _radial_positions[layer_num - 2];
    else
    {
      if (_equal_area)
      {
        const Real ring_area = (_rmax * _rmax - _rmin * _rmin) / _nr;
        current_r = std::sqrt(outer_r * outer_r - ring_area);
        outer_r = current_r;
      }
      else
      {
        if (_growth_r > 0)
          current_r -= _len * std::pow(_growth_r, layer_num - 1);
        else
          current_r -= _len * std::pow(std::abs(_growth_r), _nr - layer_num);
      }
    }

    // add node at angle = _dmin
    nodes[node_id] = mesh->add_point(Point(current_r * std::cos(_dmin * M_PI / 180.0),
                                           current_r * std::sin(_dmin * M_PI / 180.0),
                                           0.0),
                                     node_id);
    node_id++;
    for (unsigned angle_num = 1; angle_num < num_angular_nodes; ++angle_num)
    {
      const Real angle = _dmin + angle_num * dt;
      const Real x = current_r * std::cos(angle * M_PI / 180.0);
      const Real y = current_r * std::sin(angle * M_PI / 180.0);
      nodes[node_id] = mesh->add_point(Point(x, y, 0.0), node_id);
      Elem * elem = mesh->add_elem(new Quad4);
      elem->set_node(0) = nodes[node_id];
      elem->set_node(1) = nodes[node_id - 1];
      elem->set_node(2) = nodes[node_id - num_angular_nodes - 1];
      elem->set_node(3) = nodes[node_id - num_angular_nodes];
      elem->subdomain_id() = _quad_subdomain_id;
      node_id++;

      if (layer_num == _nr)
        // add outer boundary (boundary_id = 1)
        boundary_info.add_side(elem, 2, 1);
      if (layer_num == 1)
        // add inner boundary (boundary_id = 0)
        boundary_info.add_side(elem, 0, 0);
      if (!_full_annulus && angle_num == 1)
        // add tmin boundary (boundary_id = 2)
        boundary_info.add_side(elem, 1, 2);
      if (!_full_annulus && angle_num == num_angular_nodes - 1)
        // add tmin boundary (boundary_id = 3)
        boundary_info.add_side(elem, 3, 3);
    }
    if (_full_annulus)
    {
      // add element connecting to node at angle=0
      Elem * elem = mesh->add_elem(new Quad4);
      elem->set_node(0) = nodes[node_id - num_angular_nodes];
      elem->set_node(1) = nodes[node_id - 1];
      elem->set_node(2) = nodes[node_id - num_angular_nodes - 1];
      elem->set_node(3) = nodes[node_id - 2 * num_angular_nodes];
      elem->subdomain_id() = _quad_subdomain_id;

      if (layer_num == _nr)
        // add outer boundary (boundary_id = 1)
        boundary_info.add_side(elem, 2, 1);
      if (layer_num == 1)
        // add inner boundary (boundary_id = 0)
        boundary_info.add_side(elem, 0, 0);
    }
  }

  // add single node at origin, if relevant
  if (_rmin == 0.0)
  {
    nodes[node_id] = mesh->add_point(Point(0.0, 0.0, 0.0), node_id);
    boundary_info.add_node(node_id, 0); // boundary_id=0 is centre
    for (unsigned angle_num = 0; angle_num < num_angular_nodes - 1; ++angle_num)
    {
      Elem * elem = mesh->add_elem(new Tri3);
      elem->set_node(0) = nodes[node_id];
      elem->set_node(1) = nodes[node_id - num_angular_nodes + angle_num];
      elem->set_node(2) = nodes[node_id - num_angular_nodes + angle_num + 1];
      elem->subdomain_id() = _tri_subdomain_id;
    }
    if (_full_annulus)
    {
      Elem * elem = mesh->add_elem(new Tri3);
      elem->set_node(0) = nodes[node_id];
      elem->set_node(1) = nodes[node_id - 1];
      elem->set_node(2) = nodes[node_id - num_angular_nodes];
      elem->subdomain_id() = _tri_subdomain_id;
    }
  }

  boundary_info.sideset_name(0) = "rmin";
  boundary_info.sideset_name(1) = "rmax";
  boundary_info.nodeset_name(0) = "rmin";
  boundary_info.nodeset_name(1) = "rmax";
  if (!_full_annulus)
  {
    if (_radians)
    {
      boundary_info.sideset_name(2) = "tmin";
      boundary_info.sideset_name(3) = "tmax";
      boundary_info.nodeset_name(2) = "tmin";
      boundary_info.nodeset_name(3) = "tmax";
    }
    else
    {
      boundary_info.sideset_name(2) = "dmin";
      boundary_info.sideset_name(3) = "dmax";
      boundary_info.nodeset_name(2) = "dmin";
      boundary_info.nodeset_name(3) = "dmax";
    }
  }

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
