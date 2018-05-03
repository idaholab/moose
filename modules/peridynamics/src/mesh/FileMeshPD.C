//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshPD.h"
#include "MooseMesh.h"
#include "stdio.h"

#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/edge_edge2.h"

registerMooseObject("PeridynamicsApp", FileMeshPD);

template <>
InputParameters
validParams<FileMeshPD>()
{
  InputParameters params = validParams<MeshBasePD>();
  params.addClassDescription("Class for generating peridynamic mesh from finite element mesh");

  params.addRequiredParam<MeshFileName>("file", "Name of the mesh file (must be exodusII file)");

  return params;
}

FileMeshPD::FileMeshPD(const InputParameters & parameters) : MeshBasePD(parameters) {}

FileMeshPD::~FileMeshPD() { delete _fe_mesh; }

std::unique_ptr<MooseMesh>
FileMeshPD::safeClone() const
{
  return libmesh_make_unique<FileMeshPD>(*this);
}

void
FileMeshPD::init()
{
  // read the temporary mesh from Exodus file
  std::string file_name = getParam<MeshFileName>("file");
  MooseUtils::checkFileReadable(file_name);
  _fe_mesh = new SerialMesh(_communicator);
  ExodusII_IO * exodusII_io = new ExodusII_IO(*_fe_mesh);
  exodusII_io->read(file_name);
  _fe_mesh->allow_renumbering(false);
  _fe_mesh->prepare_for_use(/*true*/);

  // build neighborlist for _fe_mesh elements
  _fe_mesh->find_neighbors();

  // initialize unconventional mesh data for PD mesh
  _dim = _fe_mesh->mesh_dimension();
  _total_nodes = _fe_mesh->n_elem();
  _pdnode.resize(_total_nodes);
  _node_neighbors.resize(_total_nodes);
  _node_n_nearest_neighbors.resize(_total_nodes);
  _node_bonds.resize(_total_nodes);
  _dg_nodeinfo.resize(_total_nodes);
  _dg_bond_volumesum.resize(_total_nodes);
  _dg_node_volumesum.resize(_total_nodes);

  // loop through all fe elements to generate PD nodes structure
  for (MeshBase::element_iterator it = _fe_mesh->elements_begin(); it != _fe_mesh->elements_end();
       ++it)
  {
    Elem * fe_elem = *it;
    // calculate the mesh_spacing as average distance between _fe_mesh element with its neighbors
    unsigned int nneighbors = 0;
    Real spacing = 0.0;
    for (unsigned int i = 0; i < fe_elem->n_neighbors(); ++i)
      if (fe_elem->neighbor(i) != NULL)
      {
        spacing += (fe_elem->centroid() - fe_elem->neighbor(i)->centroid()).norm();
        nneighbors += 1;
      }
    _pdnode[fe_elem->id()].coord = fe_elem->centroid();
    _pdnode[fe_elem->id()].mesh_spacing = spacing / nneighbors;
    _pdnode[fe_elem->id()].horizon = MeshBasePD::computeHorizon(spacing / nneighbors);
    _pdnode[fe_elem->id()].volume = fe_elem->volume();
    _pdnode[fe_elem->id()].volumesum = 0.0;
    _pdnode[fe_elem->id()].blockID = fe_elem->subdomain_id();
  }

  // search node neighbors
  MeshBasePD::findNodeNeighbor();

  // setup node info for deformation gradient
  MeshBasePD::setupDGNodeInfo();

  _total_bonds = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    _total_bonds += _node_neighbors[i].size();
  _total_bonds /= 2;

  unsigned int k = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    for (unsigned int j = 0; j < _node_neighbors[i].size(); ++j)
      if (_node_neighbors[i][j] > i)
      {
        // build the bond list for each node
        _node_bonds[i].push_back(k);
        _node_bonds[_node_neighbors[i][j]].push_back(k);
        ++k;
      }

  MooseMesh::init();
}

void
FileMeshPD::buildMesh()
{
  // initialize PD mesh
  UnstructuredMesh & pd_mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  pd_mesh.clear();
  pd_mesh.set_mesh_dimension(_dim);
  pd_mesh.set_spatial_dimension(_dim);
  BoundaryInfo & pd_boundary_info = pd_mesh.get_boundary_info();
  pd_mesh.reserve_nodes(_total_nodes);
  pd_mesh.reserve_elem(_total_bonds);

  // loop through all pd_nodes to generate PD mesh nodes structure
  for (unsigned int i = 0; i < _total_nodes; ++i)
    pd_mesh.add_point(_pdnode[i].coord, i);

  // generate PD mesh
  unsigned int k = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    for (unsigned int j = 0; j < _node_neighbors[i].size(); ++j)
      if (_node_neighbors[i][j] > i)
      {
        Elem * pd_elem = pd_mesh.add_elem(new Edge2);
        pd_elem->set_id() = k;
        pd_elem->set_node(0) = pd_mesh.node_ptr(i);
        pd_elem->set_node(1) = pd_mesh.node_ptr(_node_neighbors[i][j]);
        // block id for pd mesh from fe mesh
        pd_elem->subdomain_id() = _pdnode[i].blockID;
        ++k;
      }

  // convert boundary info from fe_boundary_info to pd_boundary_info
  // build element list for user specified nodal boundaries
  std::vector<dof_id_type> elems;
  std::vector<unsigned short int> sides;
  std::vector<boundary_id_type> ids;

  BoundaryInfo & fe_boundary_info = _fe_mesh->get_boundary_info();
  fe_boundary_info.build_side_list_from_node_list();
  fe_boundary_info.build_active_side_list(elems, sides, ids);

  unsigned int n = elems.size();
  // array of boundary elems
  std::vector<BndElement *> bnd_elems(n);
  // map of set of elem IDs connected to each boundary
  std::map<boundary_id_type, std::set<dof_id_type>> bnd_elem_ids;
  for (unsigned int i = 0; i < n; ++i)
  {
    bnd_elems[i] = new BndElement(_fe_mesh->elem_ptr(elems[i]), sides[i], ids[i]);
    bnd_elem_ids[ids[i]].insert(elems[i]);
  }
  // for nodeset only
  std::set<boundary_id_type> fe_node_bid(fe_boundary_info.get_node_boundary_ids());
  std::set<boundary_id_type> fe_edge_bid(fe_boundary_info.get_edge_boundary_ids());
  if (!fe_edge_bid.empty())
    mooseError("FileMeshPD currently only accepts nodesets!");

  for (std::set<boundary_id_type>::iterator bit = fe_node_bid.begin(); bit != fe_node_bid.end();
       ++bit)
  {
    pd_boundary_info.nodeset_name(*bit) = fe_boundary_info.get_nodeset_name(*bit);
    for (MeshBase::element_iterator eit = _fe_mesh->elements_begin();
         eit != _fe_mesh->elements_end();
         ++eit)
    {
      Elem * fe_elem = *eit;
      std::map<boundary_id_type, std::set<dof_id_type>>::const_iterator it =
          bnd_elem_ids.find(*bit);
      if (it != bnd_elem_ids.end())
        if (it->second.find(fe_elem->id()) != it->second.end())
          pd_boundary_info.add_node(pd_mesh.node_ptr(fe_elem->id()), *bit);
    }
  }

  // define center and right nodes, ONLY for geometry of circular cross section
  // centered at the origin
  for (unsigned int i = 0; i < _total_nodes; ++i)
  {
    Real X = (_pdnode[i].coord)(0);
    Real Y = (_pdnode[i].coord)(1);
    Real Z = (_pdnode[i].coord)(2);
    Real dis = std::sqrt(X * X + Y * Y + Z * Z);
    if (dis < 0.001)
      pd_boundary_info.add_node(pd_mesh.node_ptr(i), 100);
    if (std::abs(Y) < 0.001)
      pd_boundary_info.add_node(pd_mesh.node_ptr(i), 101);
    if (std::abs(X) < 0.001)
      pd_boundary_info.add_node(pd_mesh.node_ptr(i), 102);
    if (std::abs(Z) < 0.001)
      pd_boundary_info.add_node(pd_mesh.node_ptr(i), 103);
    pd_boundary_info.add_node(pd_mesh.node_ptr(i), 999);
  }
  pd_boundary_info.nodeset_name(100) = "CenterPoint";
  pd_boundary_info.nodeset_name(101) = "XCenterLine";  // 2D
  pd_boundary_info.nodeset_name(102) = "YCenterLine";  // 2D
  pd_boundary_info.nodeset_name(103) = "ZCenterPlane"; // 3D
  pd_boundary_info.nodeset_name(999) = "All";

  _console << "Mesh Information:" << '\n';
  _console << "  Number of Nodes:         " << _total_nodes << '\n';
  _console << "  Number of Bonds:         " << _total_bonds << '\n';
  _console << '\n';

  // prepare for use
  pd_mesh.prepare_for_use(/*skip_renumber =*/true);
}
