#include "PinMeshGenerator.h"
#include "QuadSubChannelMesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/function_base.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/cell_tet10.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad9.h"
#include "libmesh/libmesh_logging.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/remote_elem.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/point.h"
#include "libmesh/edge_edge2.h"
#include <numeric>

registerMooseObject("SubChannelApp", PinMeshGenerator);

InputParameters
PinMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addClassDescription("Creates a mesh in the location of the pin center lines");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addParam<unsigned int>("block_id", 1, "Domain Index");
  return params;
}

PinMeshGenerator::PinMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _pitch(getParam<Real>("pitch")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _block_id(getParam<unsigned int>("block_id"))
{
  Real dz = (_unheated_length_entry + _heated_length + _unheated_length_exit) / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);
}

std::unique_ptr<MeshBase>
PinMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh_base = std::move(_input);
  if (!mesh_base)
    mesh_base = buildMeshBaseObject();
  mesh_base->set_mesh_dimension(3);
  mesh_base->reserve_elem(_n_cells * (_ny - 1) * (_nx - 1));
  mesh_base->reserve_nodes((_n_cells + 1) * (_ny - 1) * (_nx - 1));
  _pin_nodes.resize((_nx - 1) * (_ny - 1));
  // number of nodes in subchannel mesh
  unsigned int node_sub = (_n_cells + 1) * _ny * _nx;
  // number of elements in subchannel mesh
  unsigned int elem_sub = _n_cells * _ny * _nx;
  // Add the points in the shape of a rectilinear grid.  The grid is regular
  // on the xy-plane with a spacing of `pitch` between points.  The grid along
  // z is also regular.  Store pointers in the _nodes
  // array so we can keep track of which points are in which pins.
  Real offset_x = (_nx - 2) * _pitch / 2.0;
  Real offset_y = (_ny - 2) * _pitch / 2.0;
  unsigned int node_id = node_sub;
  for (unsigned int iy = 0; iy < _ny - 1; iy++)
  {
    for (unsigned int ix = 0; ix < _nx - 1; ix++)
    {
      int i_node = (_nx - 1) * iy + ix;
      _pin_nodes[i_node].reserve(_n_cells);
      for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
      {
        _pin_nodes[i_node].push_back(mesh_base->add_point(
            Point(_pitch * ix - offset_x, _pitch * iy - offset_y, _z_grid[iz]), node_id++));
      }
    }
  }

  // Add the elements which in this case are 2-node edges that link each
  // subchannel's nodes vertically.
  unsigned int elem_id = elem_sub;
  for (unsigned int iy = 0; iy < _ny - 1; iy++)
  {
    for (unsigned int ix = 0; ix < _nx - 1; ix++)
    {
      for (unsigned int iz = 0; iz < _n_cells; iz++)
      {
        Elem * elem = new Edge2;
        elem->subdomain_id() = _block_id;
        elem->set_id(elem_id++);
        elem = mesh_base->add_elem(elem);
        const int indx1 = ((_n_cells + 1) * (_nx - 1)) * iy + (_n_cells + 1) * ix + iz + node_sub;
        const int indx2 =
            ((_n_cells + 1) * (_nx - 1)) * iy + (_n_cells + 1) * ix + (iz + 1) + node_sub;
        elem->set_node(0) = mesh_base->node_ptr(indx1);
        elem->set_node(1) = mesh_base->node_ptr(indx2);
      }
    }
  }

  mesh_base->subdomain_name(_block_id) = name();
  mesh_base->prepare_for_use();

  // move the meta data into QuadSubChannelMesh
  std::shared_ptr<QuadSubChannelMesh> sch_mesh =
      std::dynamic_pointer_cast<QuadSubChannelMesh>(_mesh);
  sch_mesh->_pin_nodes = _pin_nodes;
  sch_mesh->_pin_mesh_exist = true;

  return mesh_base;
}
