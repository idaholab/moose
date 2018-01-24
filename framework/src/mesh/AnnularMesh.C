//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnnularMesh.h"

#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"

template <>
InputParameters
validParams<AnnularMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRangeCheckedParam<unsigned int>(
      "nr", 1, "nr>0", "Number of elements in the radial direction");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "nt", "nt>0", "Number of elements in the angular direction");
  params.addRequiredRangeCheckedParam<Real>(
      "rmin",
      "rmin>=0.0",
      "Inner radius.  If rmin=0 then a disc mesh (with no central hole) will be created.");
  params.addRequiredParam<Real>("rmax", "Outer radius");
  params.addParam<Real>("tmin", 0.0, "Minimum angle, measured anticlockwise from x axis");
  params.addParam<Real>("tmax",
                        2 * M_PI,
                        "Maximum angle, measured anticlockwise from x axis.  If "
                        "tmin=0 and tmax=2Pi an annular mesh is created.  "
                        "Otherwise, only a sector of an annulus is created");
  params.addRangeCheckedParam<Real>(
      "growth_r", 1.0, "growth_r>0.0", "The ratio of radial sizes of successive rings of elements");
  params.addParam<SubdomainID>(
      "quad_subdomain_id", 0, "The subdomain ID given to the QUAD4 elements");
  params.addParam<SubdomainID>("tri_subdomain_id",
                               1,
                               "The subdomain ID given to the TRI3 elements "
                               "(these exist only if rmin=0, and they exist "
                               "at the center of the disc");
  params.addClassDescription("For rmin>0: creates an annular mesh of QUAD4 elements.  For rmin=0: "
                             "creates a disc mesh of QUAD4 and TRI3 elements.  Boundary sidesets "
                             "are created at rmax and rmin, and given these names.  If tmin!=0 and "
                             "tmax!=2Pi, a sector of an annulus or disc is created.  In this case "
                             "boundary sidesets are also created a tmin and tmax, and "
                             "given these names");
  return params;
}

AnnularMesh::AnnularMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _nr(getParam<unsigned int>("nr")),
    _nt(getParam<unsigned int>("nt")),
    _rmin(getParam<Real>("rmin")),
    _rmax(getParam<Real>("rmax")),
    _tmin(getParam<Real>("tmin")),
    _tmax(getParam<Real>("tmax")),
    _growth_r(getParam<Real>("growth_r")),
    _len(_growth_r == 1.0 ? (_rmax - _rmin) / _nr
                          : (_rmax - _rmin) * (1.0 - _growth_r) / (1.0 - std::pow(_growth_r, _nr))),
    _full_annulus(_tmin == 0.0 && _tmax == 2 * M_PI),
    _quad_subdomain_id(getParam<SubdomainID>("quad_subdomain_id")),
    _tri_subdomain_id(getParam<SubdomainID>("tri_subdomain_id"))
{
  // catch likely user errors
  if (_rmax <= _rmin)
    mooseError("AnnularMesh: rmax must be greater than rmin");
  if (_tmax <= _tmin)
    mooseError("AnnularMesh: tmax must be greater than tmin");
  if (_tmax - _tmin > 2 * M_PI)
    mooseError("AnnularMesh: tmax - tmin must be <= 2 Pi");
  if (_nt <= (_tmax - _tmin) / M_PI)
    mooseError("AnnularMesh: nt must be greater than (tmax - tmin) / Pi in order to avoid inverted "
               "elements");
  if (_quad_subdomain_id == _tri_subdomain_id)
    mooseError("AnnularMesh: quad_subdomain_id must not equal tri_subdomain_id");
}

Real
AnnularMesh::getMinInDimension(unsigned int component) const
{
  switch (component)
  {
    case 0:
      return -_rmax;
    case 1:
      return -_rmax;
    case 2:
      return 0.0;
    default:
      mooseError("Invalid component");
  }
}

Real
AnnularMesh::getMaxInDimension(unsigned int component) const
{
  switch (component)
  {
    case 0:
      return _rmax;
    case 1:
      return _rmax;
    case 2:
      return 0.0;
    default:
      mooseError("Invalid component");
  }
}

MooseMesh &
AnnularMesh::clone() const
{
  return *(new AnnularMesh(*this));
}

void
AnnularMesh::buildMesh()
{
  const Real dt = (_tmax - _tmin) / _nt;

  MeshBase & mesh = getMesh();
  mesh.clear();
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(2);
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

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
    const Real angle = _tmin + angle_num * dt;
    const Real x = current_r * std::cos(angle);
    const Real y = current_r * std::sin(angle);
    nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
    node_id++;
  }

  // add nodes at smaller radii, and connect them with elements
  for (unsigned layer_num = _nr; layer_num > min_nonzero_layer_num; --layer_num)
  {
    if (layer_num == 1)
      current_r = _rmin; // account for precision loss
    else
      current_r -= _len * std::pow(_growth_r, layer_num - 1);

    // add node at angle = _tmin
    nodes[node_id] = mesh.add_point(
        Point(current_r * std::cos(_tmin), current_r * std::sin(_tmin), 0.0), node_id);
    node_id++;
    for (unsigned angle_num = 1; angle_num < num_angular_nodes; ++angle_num)
    {
      const Real angle = _tmin + angle_num * dt;
      const Real x = current_r * std::cos(angle);
      const Real y = current_r * std::sin(angle);
      nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
      Elem * elem = mesh.add_elem(new Quad4);
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
      Elem * elem = mesh.add_elem(new Quad4);
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
    nodes[node_id] = mesh.add_point(Point(0.0, 0.0, 0.0), node_id);
    boundary_info.add_node(node_id, 0); // boundary_id=0 is centre
    for (unsigned angle_num = 0; angle_num < num_angular_nodes - 1; ++angle_num)
    {
      Elem * elem = mesh.add_elem(new Tri3);
      elem->set_node(0) = nodes[node_id];
      elem->set_node(1) = nodes[node_id - num_angular_nodes + angle_num];
      elem->set_node(2) = nodes[node_id - num_angular_nodes + angle_num + 1];
      elem->subdomain_id() = _tri_subdomain_id;
    }
    if (_full_annulus)
    {
      Elem * elem = mesh.add_elem(new Tri3);
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
    boundary_info.sideset_name(2) = "tmin";
    boundary_info.sideset_name(3) = "tmax";
    boundary_info.nodeset_name(2) = "tmin";
    boundary_info.nodeset_name(3) = "tmax";
  }

  mesh.prepare_for_use(false);
}
