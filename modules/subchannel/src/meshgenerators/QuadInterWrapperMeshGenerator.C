/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "QuadInterWrapperMeshGenerator.h"
#include "QuadInterWrapperMesh.h"
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

registerMooseObject("SubChannelApp", QuadInterWrapperMeshGenerator);

InputParameters
QuadInterWrapperMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Creates a mesh for the inter-wrapper in the location of the modeled "
                             "reactor section centroid");
  params.addRequiredParam<Real>("assembly_pitch", "Pitch [m]");
  params.addRequiredParam<Real>("assembly_side_x",
                                "Outer side lengths of assembly in x [m] - including duct");
  params.addRequiredParam<Real>("assembly_side_y",
                                "Outer side lengths of assembly in y [m] - including duct");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addParam<Real>("Kij", 0.5, "Lateral form loss coefficient [-]");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addRequiredParam<unsigned int>("nx", "Number of assemblies in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of assemblies in the y direction [-]");
  params.addRequiredParam<Real>("side_bypass",
                                "Extra size of the bypass for the side assemblies [m]");
  params.addParam<unsigned int>("block_id", 0, "Domain Index");
  return params;
}

QuadInterWrapperMeshGenerator::QuadInterWrapperMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _assembly_pitch(getParam<Real>("assembly_pitch")),
    _assembly_side_x(getParam<Real>("assembly_side_x")),
    _assembly_side_y(getParam<Real>("assembly_side_y")),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _kij(getParam<Real>("Kij")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _side_bypass_length(getParam<Real>("side_bypass")),
    _n_channels((_nx + 1) * (_ny + 1)),
    _n_gaps(_nx * (_ny + 1) + _ny * (_nx + 1)),
    _n_assemblies(_nx * _ny),
    _block_id(getParam<unsigned int>("block_id"))
{
  // Converting number of assemblies into number of inter-wrapper flow channels
  _nx += 1;
  _ny += 1;

  if (_nx < 2 && _ny < 2)
    mooseError(name(), ": The number of assemblies cannot be less than one in any direction. ");

  // Defining the total length from 3 axial sections
  Real L = _unheated_length_entry + _heated_length + _unheated_length_exit;

  // Defining the dz based in the total length and the specified number of axial cells
  Real dz = L / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);

  // Defining the array for axial resistances
  _k_grid.resize(_n_channels, std::vector<Real>(_n_cells + 1));

  // Defining the size of the maps
  _gap_to_chan_map.resize(_n_gaps);
  _gapnodes.resize(_n_gaps);
  _chan_to_gap_map.resize(_n_channels);
  _chan_to_pin_map.resize(_n_channels);
  _pin_to_chan_map.resize(_n_assemblies);
  _sign_id_crossflow_map.resize(_n_channels);
  _gij_map.resize(_n_gaps);

  // Defining the signs for positive and negative flows
  Real positive_flow = 1.0;
  Real negative_flow = -1.0;

  // Defining the subchannel types
  _subch_type.resize(_n_channels);
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      unsigned int i_ch = _nx * iy + ix;
      bool is_corner = (ix == 0 && iy == 0) || (ix == _nx - 1 && iy == 0) ||
                       (ix == 0 && iy == _ny - 1) || (ix == _nx - 1 && iy == _ny - 1);
      bool is_edge = (ix == 0 || iy == 0 || ix == _nx - 1 || iy == _ny - 1);

      if (is_corner)
        _subch_type[i_ch] = EChannelType::CORNER;
      else if (is_edge)
        _subch_type[i_ch] = EChannelType::EDGE;
      else
        _subch_type[i_ch] = EChannelType::CENTER;
    }
  }

  // Index the east-west gaps.
  unsigned int i_gap = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx - 1; ix++)
    {
      unsigned int i_ch = _nx * iy + ix;
      unsigned int j_ch = _nx * iy + (ix + 1);
      _gap_to_chan_map[i_gap] = {i_ch, j_ch};
      _chan_to_gap_map[i_ch].push_back(i_gap);
      _chan_to_gap_map[j_ch].push_back(i_gap);
      _sign_id_crossflow_map[i_ch].push_back(positive_flow);
      _sign_id_crossflow_map[j_ch].push_back(negative_flow);

      // make a gap size map
      if (iy == 0 || iy == _ny - 1)
        _gij_map[i_gap] = (_assembly_pitch - _assembly_side_x) / 2 + _side_bypass_length;
      else
        _gij_map[i_gap] = (_assembly_pitch - _assembly_side_x);
      ++i_gap;
    }
  }

  // Index the north-south gaps.
  for (unsigned int iy = 0; iy < _ny - 1; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      unsigned int i_ch = _nx * iy + ix;
      unsigned int j_ch = _nx * (iy + 1) + ix;
      _gap_to_chan_map[i_gap] = {i_ch, j_ch};
      _chan_to_gap_map[i_ch].push_back(i_gap);
      _chan_to_gap_map[j_ch].push_back(i_gap);
      _sign_id_crossflow_map[i_ch].push_back(positive_flow);
      _sign_id_crossflow_map[j_ch].push_back(negative_flow);

      // make a gap size map
      if (ix == 0 || ix == _nx - 1)
        _gij_map[i_gap] = (_assembly_pitch - _assembly_side_y) / 2 + _side_bypass_length;
      else
        _gij_map[i_gap] = (_assembly_pitch - _assembly_side_y);
      ++i_gap;
    }
  }

  // Make assembly to channel map
  for (unsigned int iy = 0; iy < _ny - 1; iy++)
  {
    for (unsigned int ix = 0; ix < _nx - 1; ix++)
    {
      unsigned int i_pin = (_nx - 1) * iy + ix;
      unsigned int i_chan_1 = _nx * iy + ix;
      unsigned int i_chan_2 = _nx * (iy + 1) + ix;
      unsigned int i_chan_3 = _nx * (iy + 1) + (ix + 1);
      unsigned int i_chan_4 = _nx * iy + (ix + 1);
      _pin_to_chan_map[i_pin].push_back(i_chan_1);
      _pin_to_chan_map[i_pin].push_back(i_chan_2);
      _pin_to_chan_map[i_pin].push_back(i_chan_3);
      _pin_to_chan_map[i_pin].push_back(i_chan_4);
    }
  }

  // Make channel to assembly map
  for (unsigned int iy = 0; iy < _ny; iy++) // row
  {
    for (unsigned int ix = 0; ix < _nx; ix++) // column
    {
      unsigned int i_ch = _nx * iy + ix;
      // Corners contact 1/4 of one pin
      if (iy == 0 && ix == 0)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix);
      }
      else if (iy == _ny - 1 && ix == 0)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix);
      }
      else if (iy == 0 && ix == _nx - 1)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix - 1);
      }
      else if (iy == _ny - 1 && ix == _nx - 1)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix - 1);
      }
      // Sides contact 1/4 of two pins
      else if (iy == 0)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix);
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix - 1);
      }
      else if (iy == _ny - 1)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix);
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix - 1);
      }
      else if (ix == 0)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix);
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix);
      }
      else if (ix == _nx - 1)
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix - 1);
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix - 1);
      }
      // interior contacts 1/4 of 4 pins
      else
      {
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix);
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * iy + ix - 1);
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix);
        _chan_to_pin_map[i_ch].push_back((_nx - 1) * (iy - 1) + ix - 1);
      }
    }
  }

  // Reduce reserved memory in the channel-to-gap map.
  for (auto & gap : _chan_to_gap_map)
    gap.shrink_to_fit();

  // Reduce reserved memory in the channel-to-pin map.
  for (auto & pin : _chan_to_pin_map)
    pin.shrink_to_fit();

  // Reduce reserved memory in the pin-to-channel map.
  for (auto & pin : _pin_to_chan_map)
    pin.shrink_to_fit();

  _console << "Inter-wrapper quad mesh initialized" << std::endl;
}

std::unique_ptr<MeshBase>
QuadInterWrapperMeshGenerator::generate()
{
  auto mesh_base = buildMeshBaseObject();
  BoundaryInfo & boundary_info = mesh_base->get_boundary_info();
  mesh_base->set_spatial_dimension(3);
  mesh_base->reserve_elem(_n_cells * _ny * _nx);
  mesh_base->reserve_nodes((_n_cells + 1) * _ny * _nx);
  _nodes.resize(_nx * _ny);
  // Add the points in the shape of a rectilinear grid.  The grid is regular
  // on the xy-plane with a spacing of `pitch` between points.  The grid along
  // z is irregular to account for rod spacers.  Store pointers in the _nodes
  // array so we can keep track of which points are in which channels.
  Real offset_x = (_nx - 1) * _assembly_pitch / 2.0;
  Real offset_y = (_ny - 1) * _assembly_pitch / 2.0;
  unsigned int node_id = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      int i_ch = _nx * iy + ix;
      _nodes[i_ch].reserve(_n_cells);
      for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
      {
        _nodes[i_ch].push_back(mesh_base->add_point(
            Point(_assembly_pitch * ix - offset_x, _assembly_pitch * iy - offset_y, _z_grid[iz]),
            node_id++));
      }
    }
  }

  // Add the elements which in this case are 2-node edges that link each
  // subchannel's nodes vertically.
  unsigned int elem_id = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      for (unsigned int iz = 0; iz < _n_cells; iz++)
      {
        Elem * elem = new Edge2;
        elem->subdomain_id() = _block_id;
        elem->set_id(elem_id++);
        elem = mesh_base->add_elem(elem);
        const int indx1 = ((_n_cells + 1) * _nx) * iy + (_n_cells + 1) * ix + iz;
        const int indx2 = ((_n_cells + 1) * _nx) * iy + (_n_cells + 1) * ix + (iz + 1);
        elem->set_node(0) = mesh_base->node_ptr(indx1);
        elem->set_node(1) = mesh_base->node_ptr(indx2);

        if (iz == 0)
          boundary_info.add_side(elem, 0, 0);
        if (iz == _n_cells - 1)
          boundary_info.add_side(elem, 1, 1);
      }
    }
  }

  boundary_info.sideset_name(0) = "inlet";
  boundary_info.sideset_name(1) = "outlet";
  boundary_info.nodeset_name(0) = "inlet";
  boundary_info.nodeset_name(1) = "outlet";
  mesh_base->subdomain_name(_block_id) = name();
  mesh_base->prepare_for_use();

  // move the meta data into QuadInterWrapperMesh
  auto & sch_mesh = static_cast<QuadInterWrapperMesh &>(*_mesh);
  sch_mesh._unheated_length_entry = _unheated_length_entry;

  sch_mesh._assembly_pitch = _assembly_pitch;
  sch_mesh._assembly_side_x = _assembly_side_x;
  sch_mesh._assembly_side_y = _assembly_side_y;
  sch_mesh._heated_length = _heated_length;
  sch_mesh._unheated_length_exit = _unheated_length_exit;
  sch_mesh._z_grid = _z_grid;
  sch_mesh._k_grid = _k_grid;
  sch_mesh._kij = _kij;
  sch_mesh._n_cells = _n_cells;
  sch_mesh._nx = _nx;
  sch_mesh._ny = _ny;
  sch_mesh._side_bypass_length = _side_bypass_length;
  sch_mesh._n_channels = _n_channels;
  sch_mesh._n_gaps = _n_gaps;
  sch_mesh._n_assemblies = _n_assemblies;
  sch_mesh._nodes = _nodes;
  sch_mesh._gapnodes = _gapnodes;
  sch_mesh._gap_to_chan_map = _gap_to_chan_map;
  sch_mesh._chan_to_gap_map = _chan_to_gap_map;
  sch_mesh._chan_to_pin_map = _chan_to_pin_map;
  sch_mesh._pin_to_chan_map = _pin_to_chan_map;
  sch_mesh._sign_id_crossflow_map = _sign_id_crossflow_map;
  sch_mesh._gij_map = _gij_map;
  sch_mesh._subch_type = _subch_type;

  _console << "Inter-wrapper quad mesh generated" << std::endl;

  return mesh_base;
}
