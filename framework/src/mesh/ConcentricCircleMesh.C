#include "ConcentricCircleMesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/face_quad4.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("MooseApp", ConcentricCircleMesh);

template <>
InputParameters
validParams<ConcentricCircleMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  params.addRequiredParam<Real>("unit_cell_length", "Length of a unit cell of an assembly");

  params.addRequiredParam<Real>("radius_fuel", "Radius of fuel");
  params.addRequiredParam<Real>("inner_radius_clad", "Inner radius of cladding");
  params.addRequiredParam<Real>("outer_radius_clad", "Outer radius of cladding");
  params.addRequiredParam<unsigned int>("num_sectors",
                                        "num_sectors=2n; n>=1"
                                        "Number of azimuthal sectors in each quadrant"
                                        "'num_sectors' must be an even number.");

  params.addRequiredParam<unsigned int>(
      "nr",
      "nr>=(num_sectors/2 + 1)"
      "Number of intervals in a radial direction"
      "The number of total intervals in a radial direction includes the intervals"
      "given for the square mesh at the center, and its dimension is given as 'num_sectors/2' by "
      "'num_sectors/2'.");
  params.addRequiredParam<unsigned int>("num_intervals_unit_cell",
                                        "Numer of intervals in the unit cell part.");
  // this might not be needed. just in case for now.
  // params.addRangeCheckedParam<Real>(
  //    "growth_r", 1.0, "growth_r>0.0", "The ratio of radial sizes of successive rings of
  //    elements");
  params.addParam<SubdomainID>("fuel_subdomain_id", 1, "The subdomain ID given to fuel");
  params.addParam<SubdomainID>("clad_subdomain_id", 3, "The subdomain ID given to cladding");
  params.addParam<SubdomainID>("gap_subdomain_id", 2, "The subdomain ID given to gap");
  params.addParam<SubdomainID>("moderator_subdomain_id", 4, "The subdomain ID given to moderator");

  params.addClassDescription("This ConcentricCircleMesh is to generate a concentric circle mesh "
                             "for typical LWR type fuels.");
  return params;
}

ConcentricCircleMesh::ConcentricCircleMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _unit_cell_length(getParam<Real>("unit_cell_length")),
    _radius_fuel(getParam<Real>("radius_fuel")),
    _outer_radius_clad(getParam<Real>("outer_radius_clad")),
    _inner_radius_clad(getParam<Real>("inner_radius_clad")),
    _num_sectors(getParam<unsigned int>("num_sectors")),
    _nr(getParam<unsigned int>("nr")),
    _fuel_subdomain_id(getParam<SubdomainID>("fuel_subdomain_id")),
    _clad_subdomain_id(getParam<SubdomainID>("clad_subdomain_id")),
    _gap_subdomain_id(getParam<SubdomainID>("gap_subdomain_id")),
    _moderator_subdomain_id(getParam<SubdomainID>("moderator_subdomain_id")),
    _num_intervals_unit_cell(getParam<unsigned int>("num_intervals_unit_cell"))
//_growth_r(getParam<Real>("growth_r"))
{
  if (_num_sectors % 2 != 0)
    mooseError("ConcentricCircleMesh: num_sectors must be an even number");
  if (_unit_cell_length <= _radius_fuel)
    mooseError("ConcentricCircleMesh: The radius of a fuel pin must be smaller than the length of "
               "the unit cell");
  if (_outer_radius_clad <= _radius_fuel)
    mooseError("ConcentricCircleMesh: The outer radius of a clad must be bigger than the radius of "
               "the fuel pin");
  if (_outer_radius_clad <= _inner_radius_clad)
    mooseError("ConcentricCircleMesh: The outer radius of a clad must be bigger than the inner "
               "radius of the clad");
  if (_nr < (_num_sectors / 2 + 1))
    mooseError("ConcentricCircleMesh: The number of intervals in a radial direction must be bigger "
               "than or equal to num_sectors/2 + 1");
}

std::unique_ptr<MooseMesh>
ConcentricCircleMesh::safeClone() const
{
  return libmesh_make_unique<ConcentricCircleMesh>(*this);
}

void
ConcentricCircleMesh::buildMesh()
{
  // Get the actual libMesh mesh
  MeshBase & mesh = getMesh();

  // Set dimension of mesh
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(2);
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // radius modification (for accurate criticality calculation)
  const Real d_angle = M_PI / 2 / _num_sectors;
  const Real target_area_fuel = M_PI * std::pow(_radius_fuel, 2);
  Real modified_radius_fuel =
      std::sqrt(2 * target_area_fuel / std::sin(d_angle) / _num_sectors / 4);
  const Real target_area_gap =
      (M_PI * (std::pow(_inner_radius_clad, 2) - std::pow(_radius_fuel, 2)));
  Real modified_inner_radius_clad = std::sqrt(
      target_area_gap / std::sin(d_angle) / _num_sectors / 2 + std::pow(modified_radius_fuel, 2));
  const Real target_area_clad =
      (M_PI * (std::pow(_outer_radius_clad, 2) - std::pow(_inner_radius_clad, 2)));
  Real modified_outer_radius_clad =
      std::sqrt(target_area_clad / std::sin(d_angle) / _num_sectors / 2 +
                std::pow(modified_inner_radius_clad, 2));

  _radius_fuel = modified_radius_fuel;
  _inner_radius_clad = modified_inner_radius_clad;
  _outer_radius_clad = modified_outer_radius_clad;

  // number of extra layers outside of the square.
  unsigned num_extra_layers = _nr - _num_sectors / 2;

  // number of total nodes
  unsigned num_total_nodes = std::pow(_num_sectors / 2 + 1, 2) +
                             (_num_sectors + 1) * (num_extra_layers + 2 + _num_intervals_unit_cell);

  std::vector<Node *> nodes(num_total_nodes);
  unsigned node_id = 0;

  // for adding nodes for the square at the center of the circle.
  for (unsigned i = 0; i <= _num_sectors / 2; ++i)
  {
    const Real x = i * (_radius_fuel / _nr) * std::cos(M_PI / 4);
    for (unsigned j = 0; j <= _num_sectors / 2; ++j)
    {
      const Real y = j * (_radius_fuel / _nr) * std::sin(M_PI / 4);
      nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
      node_id++;
    }
  }

  // for adding the outer notes
  Real current_radius_fuel = 0.0;

  for (unsigned layers = 0; layers < num_extra_layers; ++layers)
  {
    current_radius_fuel = (_num_sectors / 2 + layers + 1) * (_radius_fuel / _nr);
    for (unsigned num_outer_nodes = 0; num_outer_nodes <= _num_sectors; ++num_outer_nodes)
    {
      const Real x = current_radius_fuel * std::cos(num_outer_nodes * d_angle);
      const Real y = current_radius_fuel * std::sin(num_outer_nodes * d_angle);
      nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
      node_id++;
    }
  }

  // adding nodes for the gap
  for (unsigned num_outer_nodes = 0; num_outer_nodes <= _num_sectors; ++num_outer_nodes)
  {
    const Real x = _inner_radius_clad * std::cos(num_outer_nodes * d_angle);
    const Real y = _inner_radius_clad * std::sin(num_outer_nodes * d_angle);
    nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
    node_id++;
  }

  // adding nodes for the cladding
  for (unsigned num_outer_nodes = 0; num_outer_nodes <= _num_sectors; ++num_outer_nodes)
  {
    const Real x = _outer_radius_clad * std::cos(num_outer_nodes * d_angle);
    const Real y = _outer_radius_clad * std::sin(num_outer_nodes * d_angle);
    nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
    node_id++;
  }

  // adding nodes for the unit cell
  Real current_radius_moderator = 0.0;

  for (unsigned i = 0; i < _num_intervals_unit_cell; ++i)
  {
    if (i != _num_intervals_unit_cell - 1)
    {
      current_radius_moderator =
          _outer_radius_clad + (_unit_cell_length - _outer_radius_clad) / _num_intervals_unit_cell +
          i * (_unit_cell_length - _outer_radius_clad) / _num_intervals_unit_cell;
      for (unsigned num_outer_nodes = 0; num_outer_nodes <= _num_sectors; ++num_outer_nodes)
      {
        const Real x = current_radius_moderator * std::cos(num_outer_nodes * d_angle);
        const Real y = current_radius_moderator * std::sin(num_outer_nodes * d_angle);
        nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
        node_id++;
      }
    }
    else
    {
      for (unsigned j = 0; j < _num_sectors / 2 + 1; ++j)
      {
        const Real x = _unit_cell_length;
        const Real y = _unit_cell_length * std::tan(j * d_angle);
        nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
        node_id++;
      }

      for (unsigned i = 0; i < _num_sectors / 2 + 1; ++i)
      {
        const Real x = _unit_cell_length * std::cos((i + _num_sectors / 2 + 1) * d_angle) /
                       std::sin((i + _num_sectors / 2 + 1) * d_angle);
        const Real y = _unit_cell_length;
        nodes[node_id] = mesh.add_point(Point(x, y, 0.0), node_id);
        node_id++;
      }
    }
  }

  // adding elements
  int index = 0;
  int limit = 0;
  int standard = static_cast<int>(_num_sectors);

  // This is to set the limit for the index
  if (standard > 4)
  {
    int additional_term = 0;
    int counter = standard;
    while (counter > 4)
    {
      counter = counter - 2;
      additional_term = additional_term + counter;
    }
    limit = standard + additional_term;
  }
  else if (standard == 4)
  {
    limit = standard;
  }

  // adding elements in the square
  while (index <= limit)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + _num_sectors / 2 + 1];
    elem->set_node(2) = nodes[index + _num_sectors / 2 + 2];
    elem->set_node(3) = nodes[index + 1];
    elem->subdomain_id() = _fuel_subdomain_id;

    if (index < standard / 2)
      boundary_info.add_side(elem, 3, 1);
    if (index % (standard / 2 + 1) == 0)
      boundary_info.add_side(elem, 0, 2);

    index++;
    if ((index - standard / 2) % (standard / 2 + 1) == 0)
      index++;
  }

  index = (_num_sectors / 2 + 1) * (_num_sectors / 2);
  limit = (_num_sectors / 2) * (_num_sectors / 2 + 2);

  // adding elements in one outer layer of the square (right side)
  while (index < limit)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + _num_sectors / 2 + 1];
    elem->set_node(2) = nodes[index + _num_sectors / 2 + 2];
    elem->set_node(3) = nodes[index + 1];
    elem->subdomain_id() = _fuel_subdomain_id;

    if (index == (standard / 2 + 1) * (standard / 2))
      boundary_info.add_side(elem, 0, 2);

    index++;
  }

  // adding elements in one outer layer of the square (left side)
  int counter = 0;
  while (index != _num_sectors / 2)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + (_num_sectors / 2 + 1) + counter * (_num_sectors / 2 + 2)];
    elem->set_node(2) =
        nodes[index + (_num_sectors / 2 + 1) + counter * (_num_sectors / 2 + 2) + 1];
    elem->set_node(3) = nodes[index - _num_sectors / 2 - 1];
    elem->subdomain_id() = _fuel_subdomain_id;

    if (index == standard + 1)
      boundary_info.add_side(elem, 2, 1);

    index = index - _num_sectors / 2 - 1;
    counter++;
  }

  // adding elements in the rest of the mesh (fuel part)
  index = std::pow(_num_sectors / 2 + 1, 2);
  limit = static_cast<int>(num_total_nodes) - standard - 2;
  int num_nodes_boundary = std::pow(_num_sectors / 2 + 1, 2) + _num_sectors + 1;

  // adding elements in the rest of the mesh outside of fuel part.
  counter = 0;

  while (index < limit)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    elem->set_node(0) = nodes[index];
    elem->set_node(1) = nodes[index + _num_sectors + 1];
    elem->set_node(2) = nodes[index + _num_sectors + 2];
    elem->set_node(3) = nodes[index + 1];

    if (index > limit - (standard + 1) * static_cast<int>(_num_intervals_unit_cell))
      elem->subdomain_id() = _moderator_subdomain_id;
    else if (limit - (standard + 1) * (static_cast<int>(_num_intervals_unit_cell) + 1) < index &&
             index <= limit - (standard + 1) * static_cast<int>(_num_intervals_unit_cell))
      elem->subdomain_id() = _clad_subdomain_id;
    else if (limit - (standard + 1) * (static_cast<int>(_num_intervals_unit_cell) + 2) < index &&
             index <= limit - (standard + 1) * (static_cast<int>(_num_intervals_unit_cell) + 1))
      elem->subdomain_id() = _gap_subdomain_id;
    else if (index <= limit - (standard + 1) * (static_cast<int>(_num_intervals_unit_cell) + 2))
      elem->subdomain_id() = _fuel_subdomain_id;

    int const initial = std::pow(standard / 2 + 1, 2);
    int const final = std::pow(standard / 2 + 1, 2) + _num_sectors - 1;

    if ((index - initial) % (standard + 1) == 0)
      boundary_info.add_side(elem, 0, 2);
    if ((index - final) % (standard + 1) == 0)
      boundary_info.add_side(elem, 2, 1);
    if (index > limit - (standard + 1))
    {
      if (index < limit - standard + standard / 2)
        boundary_info.add_side(elem, 1, 3);
      else
        boundary_info.add_side(elem, 1, 4);
    }

    index++;
    if (index == (num_nodes_boundary + counter * (standard + 1)) - 1)
    {
      index++;
      counter++;
    }
  }

  boundary_info.sideset_name(1) = "left";
  boundary_info.sideset_name(2) = "bottom";
  boundary_info.sideset_name(3) = "right";
  boundary_info.sideset_name(4) = "top";

  mesh.prepare_for_use(false);
}
