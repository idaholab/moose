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

#include "QuadSubChannelMeshGenerator.h"
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

registerMooseObject("SubChannelApp", QuadSubChannelMeshGenerator);

InputParameters
QuadSubChannelMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Creates a mesh of 1D subchannels in a square lattice arrangement");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("rod_diameter", "Rod diameter [m]");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<std::vector<Real>>("spacer_z",
                                             "Axial location of spacers/vanes/mixing_vanes [m]");
  params.addRequiredParam<std::vector<Real>>(
      "spacer_k", "K-loss coefficient of spacers/vanes/mixing_vanes [-]");
  params.addParam<std::vector<Real>>("z_blockage",
                                     std::vector<Real>({0.0, 0.0}),
                                     "axial location of blockage (inlet, outlet) [m]");
  params.addParam<std::vector<unsigned int>>("index_blockage",
                                             std::vector<unsigned int>({0}),
                                             "index of subchannels affected by blockage");
  params.addParam<std::vector<Real>>(
      "reduction_blockage",
      std::vector<Real>({1.0}),
      "Area reduction of subchannels affected by blockage (number to muliply the area)");
  params.addParam<std::vector<Real>>("k_blockage",
                                     std::vector<Real>({0.0}),
                                     "Form loss coefficient of subchannels affected by blockage");

  params.addParam<Real>("Kij", 0.5, "Lateral form loss coefficient [-]");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  params.addRequiredParam<Real>("gap", "(Edge Pitch W = pitch/2 + rod_diameter/2 + gap) [m]");
  params.addParam<unsigned int>("block_id", 0, "Domain Index");
  return params;
}

QuadSubChannelMeshGenerator::QuadSubChannelMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _spacer_z(getParam<std::vector<Real>>("spacer_z")),
    _spacer_k(getParam<std::vector<Real>>("spacer_k")),
    _z_blockage(getParam<std::vector<Real>>("z_blockage")),
    _index_blockage(getParam<std::vector<unsigned int>>("index_blockage")),
    _reduction_blockage(getParam<std::vector<Real>>("reduction_blockage")),
    _k_blockage(getParam<std::vector<Real>>("k_blockage")),
    _kij(getParam<Real>("Kij")),
    _pitch(getParam<Real>("pitch")),
    _rod_diameter(getParam<Real>("rod_diameter")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _n_channels(_nx * _ny),
    _n_gaps((_nx - 1) * _ny + (_ny - 1) * _nx),
    _n_pins((_nx - 1) * (_ny - 1)),
    _gap(getParam<Real>("gap")),
    _block_id(getParam<unsigned int>("block_id"))
{
  if (_spacer_z.size() != _spacer_k.size())
    mooseError(name(), ": Size of vector spacer_z should be equal to size of vector spacer_k");

  if (_spacer_z.back() > _unheated_length_entry + _heated_length + _unheated_length_exit)
    mooseError(name(), ": Location of spacers should be less than the total bundle length");

  if (_z_blockage.size() != 2)
    mooseError(name(), ": Size of vector z_blockage must be 2");

  if (*max_element(_index_blockage.begin(), _index_blockage.end()) > (_n_channels - 1))
    mooseError(name(),
               ": The index of the blocked subchannel cannot be more than the max index of the "
               "subchannels");

  if (*max_element(_reduction_blockage.begin(), _reduction_blockage.end()) > 1)
    mooseError(name(), ": The area reduction of the blocked subchannels cannot be more than 1");

  if ((_index_blockage.size() > _nx * _ny) | (_reduction_blockage.size() > _nx * _ny) |
      (_k_blockage.size() > _nx * _ny))
    mooseError(name(),
               ": Size of vectors: index_blockage, reduction_blockage, k_blockage, cannot be more "
               "than the total number of subchannels");

  if ((_index_blockage.size() != _reduction_blockage.size()) |
      (_index_blockage.size() != _k_blockage.size()) |
      (_reduction_blockage.size() != _k_blockage.size()))
    mooseError(name(),
               ": Size of vectors: index_blockage, reduction_blockage, k_blockage, must be equal "
               "to eachother");

  if (_nx < 2 && _ny < 2)
    mooseError(name(),
               ": The number of subchannels cannot be less than 2 in both directions (x and y). "
               "Smallest assembly allowed is either 2X1 or 1X2. ");

  SubChannelMesh::generateZGrid(
      _unheated_length_entry, _heated_length, _unheated_length_exit, _n_cells, _z_grid);

  // Defining the total length from 3 axial sections
  Real L = _unheated_length_entry + _heated_length + _unheated_length_exit;

  // Defining the position of the spacer grid in the numerical solution array
  std::vector<int> spacer_cell;
  for (const auto & elem : _spacer_z)
    spacer_cell.emplace_back(std::round(elem * _n_cells / L));

  // Defining the arrays for axial resistances
  std::vector<Real> kgrid;
  kgrid.resize(_n_cells + 1, 0.0);
  _k_grid.resize(_n_channels, std::vector<Real>(_n_cells + 1));

  // Summing the spacer resistance to the 1D grid resistance array
  for (unsigned int index = 0; index < spacer_cell.size(); index++)
    kgrid[spacer_cell[index]] += _spacer_k[index];

  // Creating the 2D grid resistance array
  for (unsigned int i = 0; i < _n_channels; i++)
    _k_grid[i] = kgrid;

  // Add blockage resistance to the 2D grid resistane array
  Real dz = L / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
  {
    if ((dz * i >= _z_blockage.front() && dz * i <= _z_blockage.back()))
    {
      unsigned int index(0);
      for (const auto & i_ch : _index_blockage)
      {
        _k_grid[i_ch][i] += _k_blockage[index];
        index++;
      }
    }
  }

  // Defining the size of the maps
  _gap_to_chan_map.resize(_n_gaps);
  _gapnodes.resize(_n_gaps);
  _chan_to_gap_map.resize(_n_channels);
  _chan_to_pin_map.resize(_n_channels);
  _pin_to_chan_map.resize(_n_pins);
  _sign_id_crossflow_map.resize(_n_channels);
  _gij_map.resize(_n_gaps);
  _subchannel_position.resize(_n_channels);

  for (unsigned int i = 0; i < _n_channels; i++)
  {
    _subchannel_position[i].reserve(3);
    for (unsigned int j = 0; j < 3; j++)
    {
      _subchannel_position.at(i).push_back(0.0);
    }
  }

  // Defining the signs for positive and negative flows
  double positive_flow = 1.0;
  double negative_flow = -1.0;

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

      if (_n_channels == 2)
      {
        _subch_type[i_ch] = EChannelType::CENTER;
      }
      else if (_n_channels == 4)
      {
        _subch_type[i_ch] = EChannelType::CORNER;
      }
      else
      {
        if (is_corner)
          _subch_type[i_ch] = EChannelType::CORNER;
        else if (is_edge)
          _subch_type[i_ch] = EChannelType::EDGE;
        else
          _subch_type[i_ch] = EChannelType::CENTER;
      }
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
        _gij_map[i_gap] = (_pitch - _rod_diameter) / 2 + _gap;
      else
        _gij_map[i_gap] = (_pitch - _rod_diameter);
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
        _gij_map[i_gap] = (_pitch - _rod_diameter) / 2 + _gap;
      else
        _gij_map[i_gap] = (_pitch - _rod_diameter);
      ++i_gap;
    }
  }

  // Make pin to channel map
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

  // Make channel to pin map
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

      // set the subchannel positions
      Real offset_x = (_nx - 1) * _pitch / 2.0;
      Real offset_y = (_ny - 1) * _pitch / 2.0;
      _subchannel_position[i_ch][0] = _pitch * ix - offset_x;
      _subchannel_position[i_ch][1] = _pitch * iy - offset_y;
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
}

std::unique_ptr<MeshBase>
QuadSubChannelMeshGenerator::generate()
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
  Real offset_x = (_nx - 1) * _pitch / 2.0;
  Real offset_y = (_ny - 1) * _pitch / 2.0;
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
            Point(_pitch * ix - offset_x, _pitch * iy - offset_y, _z_grid[iz]), node_id++));
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

  // move the meta data into QuadSubChannelMesh
  std::shared_ptr<QuadSubChannelMesh> sch_mesh =
      std::dynamic_pointer_cast<QuadSubChannelMesh>(_mesh);
  sch_mesh->_unheated_length_entry = _unheated_length_entry;
  sch_mesh->_heated_length = _heated_length;
  sch_mesh->_unheated_length_exit = _unheated_length_exit;
  sch_mesh->_z_grid = _z_grid;
  sch_mesh->_k_grid = _k_grid;
  sch_mesh->_spacer_z = _spacer_z;
  sch_mesh->_spacer_k = _spacer_k;
  sch_mesh->_z_blockage = _z_blockage;
  sch_mesh->_index_blockage = _index_blockage;
  sch_mesh->_reduction_blockage = _reduction_blockage;
  sch_mesh->_kij = _kij;
  sch_mesh->_pitch = _pitch;
  sch_mesh->_rod_diameter = _rod_diameter;
  sch_mesh->_n_cells = _n_cells;
  sch_mesh->_nx = _nx;
  sch_mesh->_ny = _ny;
  sch_mesh->_n_channels = _n_channels;
  sch_mesh->_n_gaps = _n_gaps;
  sch_mesh->_n_pins = _n_pins;
  sch_mesh->_gap = _gap;
  sch_mesh->_nodes = _nodes;
  sch_mesh->_gapnodes = _gapnodes;
  sch_mesh->_gap_to_chan_map = _gap_to_chan_map;
  sch_mesh->_chan_to_gap_map = _chan_to_gap_map;
  sch_mesh->_chan_to_pin_map = _chan_to_pin_map;
  sch_mesh->_pin_to_chan_map = _pin_to_chan_map;
  sch_mesh->_sign_id_crossflow_map = _sign_id_crossflow_map;
  sch_mesh->_gij_map = _gij_map;
  sch_mesh->_subchannel_position = _subchannel_position;
  sch_mesh->_subch_type = _subch_type;

  return mesh_base;
}
