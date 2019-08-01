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
  params.addParam<std::vector<SubdomainID>>("convert_block_ids",
                                            "IDs of the FE mesh blocks to be converted to PD mesh");
  params.addRequiredParam<bool>(
      "retain_fe_mesh",
      "whether to retain the FE mesh or not in addition to the newly created PD mesh");

  return params;
}

MeshGeneratorPD::MeshGeneratorPD(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _has_block_id(isParamValid("convert_block_ids")),
    _retain_fe_mesh(getParam<bool>("retain_fe_mesh"))
{
  if (_has_block_id)
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("convert_block_ids");
    for (unsigned int i = 0; i < ids.size(); ++i)
      _conv_block_ids.insert(ids[i]);
  }
}

std::unique_ptr<MeshBase>
MeshGeneratorPD::generate()
{
  // get the MeshBase object this generator will be working on
  std::unique_ptr<MeshBase> old_mesh = std::move(_input);

  // STEP 1: obtain FE block(s) and elements to be converted to PD mesh

  // get the IDs of all available blocks in the input FE mesh
  std::set<SubdomainID> all_block_ids;
  for (const auto & old_elem : old_mesh->element_ptr_range())
    all_block_ids.insert(old_elem->subdomain_id());
  // categorize mesh blocks into converted and unconverted blocks
  std::set<SubdomainID> unconv_block_ids;
  if (_has_block_id)
    std::set_difference(all_block_ids.begin(),
                        all_block_ids.end(),
                        _conv_block_ids.begin(),
                        _conv_block_ids.end(),
                        std::inserter(unconv_block_ids, unconv_block_ids.begin()));
  else // if no block ids provided by user, by default, convert all FE mesh to PD mesh
    _conv_block_ids = all_block_ids;

  // save IDs of converted FE elems
  std::vector<dof_id_type> conv_elem_ids;
  // retained FE mesh and unconverted FE mesh, if any
  std::set<dof_id_type> fe_nodes_ids;
  std::set<dof_id_type> fe_elems_ids;
  for (const auto & old_elem : old_mesh->element_ptr_range())
    if (_conv_block_ids.count(old_elem->subdomain_id())) // record converted FE elem IDs
    {
      conv_elem_ids.push_back(old_elem->id());
      if (_retain_fe_mesh) // save converted elems and their nodes if retained
      {
        fe_elems_ids.insert(old_elem->id());
        for (unsigned int i = 0; i < old_elem->n_nodes(); ++i)
          fe_nodes_ids.insert(old_elem->node_id(i));
      }
    }
    else // save unconverted elements and their nodes
    {
      fe_elems_ids.insert(old_elem->id());
      for (unsigned int i = 0; i < old_elem->n_nodes(); ++i)
        fe_nodes_ids.insert(old_elem->node_id(i));
    }

  // number of FE elements and nodes in the old mesh to be saved in the new mesh
  dof_id_type n_fe_nodes = fe_nodes_ids.size();
  dof_id_type n_fe_elems = fe_elems_ids.size();

  // STEP 2: generate PD data based on to-be converted FE mesh and prepare for new mesh

  PeridynamicsMesh & pd_mesh = dynamic_cast<PeridynamicsMesh &>(*_mesh);
  // generate PD node data
  pd_mesh.createPeridynamicsMeshData(*old_mesh, conv_elem_ids);

  // number of PD elements and nodes to be created
  dof_id_type n_pd_nodes = pd_mesh.nPDNodes();
  dof_id_type n_pd_bonds = pd_mesh.nPDBonds();

  // initialize an empty new mesh object
  std::unique_ptr<MeshBase> new_mesh = _mesh->buildMeshBaseObject();
  new_mesh->clear();
  // set new mesh dimension
  new_mesh->set_mesh_dimension(old_mesh->mesh_dimension());
  new_mesh->set_spatial_dimension(old_mesh->spatial_dimension());
  // reserve elements and nodes for the new mesh
  new_mesh->reserve_nodes(n_pd_bonds + n_fe_elems);
  new_mesh->reserve_elem(n_pd_nodes + n_fe_nodes);

  // STEP 3: add points of PD data and FE mesh (retained and/or unconverted, if any) to new mesh

  // save PD nodes to new mesh first
  unsigned int new_node_id = 0;
  for (unsigned int i = 0; i < n_pd_nodes; ++i)
  {
    new_mesh->add_point(old_mesh->elem_ptr(conv_elem_ids[i])->centroid(), new_node_id);

    ++new_node_id;
  }
  // then save both retained and unconverted FE nodes, if any, to the new mesh
  // map of IDs of the same point in old and new meshes
  std::map<dof_id_type, dof_id_type> fe_nodes_map;
  for (const auto & nid : fe_nodes_ids)
  {
    new_mesh->add_point(*old_mesh->node_ptr(nid), new_node_id);
    fe_nodes_map.insert(std::make_pair(old_mesh->node_ptr(nid)->id(), new_node_id));

    ++new_node_id;
  }

  // STEP 4: generate PD elem, and FE elem using retained and/or unconverted mesh, if any

  // generate and save PD elements first to new mesh
  unsigned int new_elem_id = 0;
  for (unsigned int i = 0; i < n_pd_nodes; ++i)
  {
    std::vector<dof_id_type> pd_node_neighbors = pd_mesh.getNeighbors(i); // neighbors of a PD node
    for (unsigned int j = 0; j < pd_node_neighbors.size(); ++j)
      if (pd_node_neighbors[j] > i)
      {
        Elem * new_elem = new Edge2;
        new_elem->set_id(new_elem_id);
        new_elem->subdomain_id() = pd_mesh.getNodeBlockID(i); // block id for PD mesh from FE mesh
        new_elem = new_mesh->add_elem(new_elem);
        new_elem->set_node(0) = new_mesh->node_ptr(i);
        new_elem->set_node(1) = new_mesh->node_ptr(pd_node_neighbors[j]);

        ++new_elem_id;
      }
  }

  // then save FE elements, if any, to new mesh
  std::map<dof_id_type, dof_id_type> fe_elems_map; // IDs of the same elem in the old and new meshes
  for (const auto & eid : fe_elems_ids)
  {
    Elem * old_elem = old_mesh->elem_ptr(eid);
    Elem * new_elem = Elem::build(old_elem->type()).release();
    new_elem->set_id(new_elem_id);
    new_elem->subdomain_id() = old_elem->subdomain_id();
    new_elem = new_mesh->add_elem(new_elem);
    for (unsigned int j = 0; j < old_elem->n_nodes(); ++j)
      new_elem->set_node(j) = new_mesh->node_ptr(fe_nodes_map.at(old_elem->node_ptr(j)->id()));

    fe_elems_map.insert(std::make_pair(old_elem->id(), new_elem_id));

    ++new_elem_id;
  }

  // STEP 5: convert old boundary_info to new boundary_info

  BoundaryInfo & new_boundary_info = new_mesh->get_boundary_info();
  BoundaryInfo & old_boundary_info = old_mesh->get_boundary_info();

  // peridynamics doesnot accept edgesets and facesets
  if (old_boundary_info.n_edge_conds())
    mooseError("PeridynamicsMesh doesn't support edgesets!");

  // check the existence of nodesets, if exist, build sidesets
  if (old_boundary_info.n_nodeset_conds())
    old_boundary_info.build_side_list_from_node_list();

  // first, create a tuple to collect all sidesets (including those converted from nodesets) in the
  // old mesh
  auto old_bc_tuples = old_boundary_info.build_side_list();
  // 0: element ID, 1: side ID, 2: boundary ID
  // map of set of elem IDs connected to each boundary in the old mesh
  std::map<boundary_id_type, std::set<dof_id_type>> old_bnd_elem_ids;
  // map of set of side ID for each elem in the old mesh
  std::map<dof_id_type, std::map<boundary_id_type, dof_id_type>> old_bnd_elem_side_ids;
  for (const auto & bct : old_bc_tuples)
  {
    old_bnd_elem_ids[std::get<2>(bct)].insert(std::get<0>(bct));
    old_bnd_elem_side_ids[std::get<0>(bct)].insert(
        std::make_pair(std::get<2>(bct), std::get<1>(bct)));
  }

  // next, convert element lists in old mesh to PD nodesets in new mesh
  std::set<boundary_id_type> old_side_bid(old_boundary_info.get_side_boundary_ids());

  // loop through all old FE sideset boundaries
  for (const auto & sbidit : old_side_bid)
  {
    // create PD nodeset in new mesh based on converted FE element list in old mesh
    for (const auto & beidit : old_bnd_elem_ids[sbidit])
    {
      std::vector<dof_id_type>::iterator itr =
          std::find(conv_elem_ids.begin(), conv_elem_ids.end(), beidit);
      if (itr != conv_elem_ids.end()) // for converted FE mesh
      {
        // save corresponding boundaries on converted FE mesh to PD nodes
        new_boundary_info.add_node(new_mesh->node_ptr(std::distance(conv_elem_ids.begin(), itr)),
                                   sbidit + 1000);
        new_boundary_info.nodeset_name(sbidit + 1000) =
            "pd_" + old_boundary_info.get_sideset_name(sbidit) + Moose::stringify(sbidit);

        if (_retain_fe_mesh) // if retained, copy the corresponding boundaries, if any, to new mesh
                             // from old mesh
        {
          new_boundary_info.add_side(new_mesh->elem_ptr(fe_elems_map[beidit]),
                                     old_bnd_elem_side_ids[beidit][sbidit],
                                     sbidit);
          new_boundary_info.sideset_name(sbidit) = old_boundary_info.get_sideset_name(sbidit);
        }
      }
      else // for unconverted FE mesh, if any, copy the corresponding boundaries to new mesh from
           // old mesh
      {
        new_boundary_info.add_side(new_mesh->elem_ptr(fe_elems_map[beidit]),
                                   old_bnd_elem_side_ids[beidit][sbidit],
                                   sbidit);
        new_boundary_info.sideset_name(sbidit) = old_boundary_info.get_sideset_name(sbidit);
      }
    }
  }

  // create a nodeset to include all PD nodes in the new mesh
  for (unsigned int i = 0; i < n_pd_nodes; ++i)
    new_boundary_info.add_node(new_mesh->node_ptr(i), 999);
  new_boundary_info.nodeset_name(999) = "pd_all";

  old_mesh.reset(); // destroy the old_mesh unique_ptr

  return dynamic_pointer_cast<MeshBase>(new_mesh);
}
