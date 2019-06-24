//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshGeneratorPD.h"
#include "PeridynamicsMesh.h"
#include "CastUniquePointer.h"

#include "libmesh/edge_edge2.h"

registerMooseObject("PeridynamicsApp", MeshGeneratorPD);

template <>
InputParameters
validParams<MeshGeneratorPD>()
{
  InputParameters params = validParams<MeshGenerator>();
  params.addClassDescription("Mesh generator class to convert FE mesh to Peridynamics mesh");

  params.addRequiredParam<MeshGeneratorName>("input",
                                             "The mesh based on which PD mesh will be created");
  params.addRequiredParam<bool>(
      "retain_fe_mesh",
      "whether to retain the FE mesh or not in addition to the newly created PD mesh");

  return params;
}

MeshGeneratorPD::MeshGeneratorPD(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _retain_fe_mesh(getParam<bool>("retain_fe_mesh"))
{
}

std::unique_ptr<MeshBase>
MeshGeneratorPD::generate()
{
  // TODO: generate PD mesh depending on the value of _retain_fe_mesh
  // for now, no FE mesh will be retained after its conversion to PD mesh

  // get the MeshBase object this generator will be working on
  std::unique_ptr<MeshBase> fe_mesh = std::move(_input);

  PeridynamicsMesh & pd_data_mesh = dynamic_cast<PeridynamicsMesh &>(*_mesh);
  // call PD mesh class to generate PD specific data
  pd_data_mesh.createExtraPeridynamicsMeshData(*fe_mesh);

  // fetch data from created extra PD mesh data
  unsigned int dim = pd_data_mesh.dimension();
  dof_id_type n_pdnodes = pd_data_mesh.nPDNodes();
  dof_id_type n_pdbonds = pd_data_mesh.nPDBonds();

  auto pd_mesh = _mesh->buildMeshBaseObject();

  pd_mesh->clear();
  pd_mesh->set_mesh_dimension(dim);
  pd_mesh->set_spatial_dimension(dim);
  pd_mesh->reserve_nodes(n_pdnodes);
  pd_mesh->reserve_elem(n_pdbonds);

  // loop through all pd_nodes to generate PD mesh nodes structure
  for (unsigned int i = 0; i < n_pdnodes; ++i)
    pd_mesh->add_point(fe_mesh->elem_ptr(i)->centroid(), i);

  // generate PD mesh using created extra PD data
  unsigned int k = 0;
  for (unsigned int i = 0; i < n_pdnodes; ++i)
  {
    // get neighbors of current pdnode
    std::vector<dof_id_type> pdnode_neighbors = pd_data_mesh.getNeighbors(i);
    for (unsigned int j = 0; j < pdnode_neighbors.size(); ++j)
      if (pdnode_neighbors[j] > i)
      {
        Elem * pd_elem = pd_mesh->add_elem(new Edge2);
        pd_elem->set_id() = k;
        pd_elem->set_node(0) = pd_mesh->node_ptr(i);
        pd_elem->set_node(1) = pd_mesh->node_ptr(pdnode_neighbors[j]);
        // block id for PD mesh from FE mesh
        pd_elem->subdomain_id() = pd_data_mesh.getNodeBlockID(i);
        ++k;
      }
  }

  // now, convert FE boundary_info to PD boundary_info
  // build element lists from user specified nodesets and sidesets
  BoundaryInfo & pd_boundary_info = pd_mesh->get_boundary_info();
  BoundaryInfo & fe_boundary_info = fe_mesh->get_boundary_info();

  // peridynamics doesnot accept edgesets
  if (fe_boundary_info.n_edge_conds())
    mooseError("PeridynamicsMesh doesn't accept edgesets!");

  // check the existence of nodesets, if exist, build sidesets and element lists
  if (fe_boundary_info.n_nodeset_conds())
    fe_boundary_info.build_side_list_from_node_list();

  // in case there are only sidesets
  fe_boundary_info.build_node_list_from_side_list();

  auto bc_tuples = fe_boundary_info.build_active_side_list();
  // 0: element ID, 1: side ID, 2: boundary ID
  // map of set of elem IDs connected to each boundary
  std::map<boundary_id_type, std::set<dof_id_type>> bnd_elem_ids;
  for (const auto & t : bc_tuples)
    bnd_elem_ids[std::get<2>(t)].insert(std::get<0>(t));

  // so far, we have built FE element lists for corresponding nodesets and sidesets
  // next, we need to convert these FE element lists to PD nodesets
  std::set<boundary_id_type> fe_node_bid(fe_boundary_info.get_node_boundary_ids());

  // loop through all FE nodeset or sideset boundaries
  for (std::set<boundary_id_type>::iterator bit = fe_node_bid.begin(); bit != fe_node_bid.end();
       ++bit)
  {
    // use the same nodeset or sideset name for PD boundaries
    pd_boundary_info.nodeset_name(*bit) = fe_boundary_info.get_nodeset_name(*bit);
    // convert FE element list corresponding to current boundary ID to PD nodeset
    std::set<dof_id_type> elem_ids = bnd_elem_ids[*bit];
    for (std::set<dof_id_type>::iterator sit = elem_ids.begin(); sit != elem_ids.end(); ++sit)
      pd_boundary_info.add_node(pd_mesh->node_ptr(*sit), *bit);
  }

  // create a nodeset to include all PD nodes
  for (unsigned int i = 0; i < n_pdnodes; ++i)
    pd_boundary_info.add_node(pd_mesh->node_ptr(i), 9999);
  pd_boundary_info.nodeset_name(9999) = "all";

  fe_mesh.reset();

  return dynamic_pointer_cast<MeshBase>(pd_mesh);
}
