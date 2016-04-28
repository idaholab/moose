/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MortarPeriodicMesh.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/edge_edge2.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

template<>
InputParameters validParams<MortarPeriodicMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  MooseEnum dims("2=2 3"); // only 2d for now
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated"); // Make this parameter required

  params.addRangeCheckedParam<int>("nx", 1, "nx>0", "Number of elements in the X direction");
  params.addRangeCheckedParam<int>("ny", 1, "ny>=0", "Number of elements in the Y direction");
  params.addRangeCheckedParam<int>("nz", 1, "nz>=0", "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<unsigned int>("mortar_block_x", 1, "Block id of the mortar interface perpendicular to the x-axis");
  params.addParam<unsigned int>("mortar_block_y", 2, "Block id of the mortar interface perpendicular to the y-axis");
  params.addParam<unsigned int>("mortar_block_z", 3, "Block id of the mortar interface perpendicular to the z-axis");

  params.addParamNamesToGroup("dim", "Main");

  return params;
}

MortarPeriodicMesh::MortarPeriodicMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax")),
    _block_x(getParam<unsigned int>("mortar_block_x")),
    _block_y(getParam<unsigned int>("mortar_block_y")),
    _block_z(getParam<unsigned int>("mortar_block_z")),
    _us_mesh(dynamic_cast<UnstructuredMesh &>(getMesh()))
{
}

MortarPeriodicMesh::MortarPeriodicMesh(const MortarPeriodicMesh & other_mesh) :
    MooseMesh(other_mesh),
    _dim(other_mesh._dim),
    _nx(other_mesh._nx),
    _ny(other_mesh._ny),
    _nz(other_mesh._nz),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax")),
    _us_mesh(dynamic_cast<UnstructuredMesh &>(getMesh()))
{
}

MortarPeriodicMesh::~MortarPeriodicMesh()
{
}

MooseMesh &
MortarPeriodicMesh::clone() const
{
  return *(new MortarPeriodicMesh(*this));
}

void
MortarPeriodicMesh::buildMesh()
{
  // hard code teh elem type for now (otherwise I have to duplicate too much stuff from libmesh)
  ElemType elem_type_2D = Utility::string_to_enum<ElemType>("QUAD4");
  ElemType elem_type_3D = Utility::string_to_enum<ElemType>("HEX8");

  switch (_dim)
  {
    case 2:
      // build the main mesh
      MeshTools::Generation::build_square(dynamic_cast<UnstructuredMesh &>(getMesh()),
                                          _nx, _ny,
                                          _xmin, _xmax,
                                          _ymin, _ymax,
                                          elem_type_2D);

      // build the right mortar interface
      addLineMesh(_ny, _xmax * 1.01, _ymin, _xmax * 1.01, _ymax, _block_x);

      // build the top mortar interface
      addLineMesh(_nx, _xmin, _ymax * 1.01, _xmax, _ymax * 1.01, _block_y);
      break;

    case 3: // not yet implemented
    default:
      mooseError("Only 2D meshes are supported.");
  }
  _us_mesh.prepare_for_use(/*skip_renumber =*/ false);
}

void
MortarPeriodicMesh::addLineMesh(unsigned int nelem, Real x0, Real y0, Real x1, Real y1, subdomain_id_type id)
{
  dof_id_type max_node_id = _us_mesh.max_node_id() + 1;

  // add nodes
  for (unsigned int i = 0; i <= nelem; ++i)
    _us_mesh.add_point (Point(static_cast<Real>(i) / nelem * (x1 - x0) + x0,
                              static_cast<Real>(i) / nelem * (y1 - y0) + y0,
                              0), max_node_id + i);

  // add edges
  for (unsigned int i = 0; i < nelem; ++i)
  {
    Elem * elem = _us_mesh.add_elem(new Edge2);
    elem->subdomain_id() = id;
    elem->set_node(0) = _us_mesh.node_ptr(max_node_id + i);
    elem->set_node(1) = _us_mesh.node_ptr(max_node_id + i + 1);
  }
}
