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
#include "libmesh/face_tri3.h"
#include "libmesh/cell_tet4.h"

registerMooseObject("PeridynamicsApp", MeshGeneratorPD);

InputParameters
MeshGeneratorPD::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Mesh generator class to convert FE mesh to Peridynamics mesh");

  params.addRequiredParam<MeshGeneratorName>("input",
                                             "The mesh based on which PD mesh will be created");
  params.addParam<std::vector<SubdomainID>>("convert_block_ids",
                                            "IDs of the FE mesh blocks to be converted to PD mesh");
  params.addParam<std::vector<SubdomainID>>(
      "non_convert_block_ids",
      "IDs of the FE mesh blocks to not be converted to PD mesh. This should only be used when the "
      "number of to-be-converted FE blocks is considerable.");
  params.addRequiredParam<bool>(
      "retain_fe_mesh", "Whether to retain the FE mesh or not after conversion into PD mesh");
  params.addParam<bool>("single_converted_block",
                        false,
                        "Whether to combine converted PD mesh blocks into a single block. This is "
                        "used when all PD blocks have the same properties");
  params.addParam<bool>(
      "construct_peridynamics_sideset",
      false,
      "Whether to construct peridynamics sidesets based on the sidesets in original FE mesh");
  params.addParam<std::vector<SubdomainID>>(
      "connect_block_id_pairs",
      "List of block id pairs between which will be connected via interfacial bonds");
  params.addParam<std::vector<SubdomainID>>(
      "non_connect_block_id_pairs", "List of block pairs between which will not be connected");
  params.addParam<bool>("single_interface_block",
                        false,
                        "Whether to combine interface blocks into a single block. This is used "
                        "when all interface blocks have the same properties");

  return params;
}

MeshGeneratorPD::MeshGeneratorPD(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _has_conv_blk_ids(isParamValid("convert_block_ids")),
    _has_non_conv_blk_ids(isParamValid("non_convert_block_ids")),
    _retain_fe_mesh(getParam<bool>("retain_fe_mesh")),
    _single_converted_blk(getParam<bool>("single_converted_block")),
    _construct_pd_sideset(getParam<bool>("construct_peridynamics_sideset")),
    _has_connect_blk_id_pairs(isParamValid("connect_block_id_pairs")),
    _has_non_connect_blk_id_pairs(isParamValid("non_connect_block_id_pairs")),
    _single_interface_blk(getParam<bool>("single_interface_block"))
{
  if (_has_conv_blk_ids && _has_non_conv_blk_ids)
    mooseError("Please specifiy either 'convert_block_ids' or 'non_convert_block_ids'!");

  if (_has_non_conv_blk_ids)
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("non_convert_block_ids");
    for (unsigned int i = 0; i < ids.size(); ++i)
      _non_conv_blk_ids.insert(ids[i]);
  }

  if (_has_conv_blk_ids)
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("convert_block_ids");
    for (unsigned int i = 0; i < ids.size(); ++i)
      _conv_blk_ids.insert(ids[i]);
  }

  if (_has_connect_blk_id_pairs && _has_non_connect_blk_id_pairs)
    mooseError("Please specifiy either 'connect_block_id_pairs' or 'non_connect_block_id_pairs'!");

  _connect_blk_id_pairs.clear();
  if (_has_connect_blk_id_pairs)
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("connect_block_id_pairs");

    if (ids.size() % 2 != 0)
      mooseError("Input parameter 'connect_block_id_pairs' must contain even number of entries!");

    const unsigned int pairs = ids.size() / 2;
    for (unsigned int i = 0; i < pairs; ++i) // consider the renumbering of IDs of converted blocks
      _connect_blk_id_pairs.insert(std::make_pair(ids[2 * i] + 1000, ids[2 * i + 1] + 1000));
  }

  _non_connect_blk_id_pairs.clear();
  if (_has_non_connect_blk_id_pairs)
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("non_connect_block_id_pairs");

    if (ids.size() % 2 != 0)
      mooseError(
          "Input parameter 'non_connect_block_id_pairs' must contain even number of entries!");

    const unsigned int pairs = ids.size() / 2;
    for (unsigned int i = 0; i < pairs; ++i) // consider the renumbering of IDs of converted blocks
      _non_connect_blk_id_pairs.insert(std::make_pair(ids[2 * i] + 1000, ids[2 * i + 1] + 1000));
  }
}

std::unique_ptr<MeshBase>
MeshGeneratorPD::generate()
{
  // get the MeshBase object this generator will be working on
  std::unique_ptr<MeshBase> old_mesh = std::move(_input);

  // STEP 1: obtain FE block(s) and elements to be converted to PD mesh

  // get the IDs of all available blocks in the input FE mesh
  std::set<SubdomainID> all_blk_ids;
  for (const auto & old_elem : old_mesh->element_ptr_range())
    all_blk_ids.insert(old_elem->subdomain_id());

  // the maximum FE block ID, which will be used in determine the block ID for interfacial bond in
  // the case of single interface block
  const unsigned int max_fe_blk_id = *all_blk_ids.rbegin();

  // categorize mesh blocks into converted and non-converted blocks
  if (_has_conv_blk_ids)
    std::set_difference(all_blk_ids.begin(),
                        all_blk_ids.end(),
                        _conv_blk_ids.begin(),
                        _conv_blk_ids.end(),
                        std::inserter(_non_conv_blk_ids, _non_conv_blk_ids.begin()));
  else if (_has_non_conv_blk_ids)
    std::set_difference(all_blk_ids.begin(),
                        all_blk_ids.end(),
                        _non_conv_blk_ids.begin(),
                        _non_conv_blk_ids.end(),
                        std::inserter(_conv_blk_ids, _conv_blk_ids.begin()));
  else // if no block ids provided by user, by default, convert all FE mesh to PD mesh
    _conv_blk_ids = all_blk_ids;

  // the minimum converted FE block ID, which will be used to assign block ID for non-interfacial
  // bond in the case of combine converted blocks
  const unsigned int min_converted_fe_blk_id = *_conv_blk_ids.begin();

  // IDs of to-be-converted FE elems
  std::set<dof_id_type> conv_elem_ids;
  // retained FE mesh and non-converted FE mesh, if any
  std::set<dof_id_type> fe_nodes_ids;
  std::set<dof_id_type> fe_elems_ids;
  for (const auto & old_elem : old_mesh->element_ptr_range())
    if (_conv_blk_ids.count(old_elem->subdomain_id())) // record to-be-converted FE elem IDs
    {
      conv_elem_ids.insert(old_elem->id());
      if (_retain_fe_mesh) // save converted elems and their nodes if need to be retained
      {
        fe_elems_ids.insert(old_elem->id());
        for (unsigned int i = 0; i < old_elem->n_nodes(); ++i)
          fe_nodes_ids.insert(old_elem->node_id(i));
      }
    }
    else // save non-converted elements and their nodes
    {
      fe_elems_ids.insert(old_elem->id());
      for (unsigned int i = 0; i < old_elem->n_nodes(); ++i)
        fe_nodes_ids.insert(old_elem->node_id(i));
    }

  // number of FE elements and nodes in the old mesh to be saved in the new mesh
  dof_id_type n_fe_nodes = fe_nodes_ids.size();
  dof_id_type n_fe_elems = fe_elems_ids.size();
  dof_id_type n_phantom_elems = 0;

  // determine the number of phantom elements to be generated in the new mesh based on sideset in
  // old mesh
  BoundaryInfo & old_boundary_info = old_mesh->get_boundary_info();

  // save the IDs of FE sidesets (excluding constructed from nodesets) in old mesh
  std::set<boundary_id_type> fe_sbnd_ids = old_boundary_info.get_side_boundary_ids();
  // determine number of FE side elements, the number of actual phantom elements is less than or
  // equal to the number of FE side elements, this number is used to reserve number of elements
  // in the new mesh only
  std::map<boundary_id_type, std::set<dof_id_type>> fe_sbnd_elem_ids;
  auto fe_sbc_tuples = old_boundary_info.build_side_list();
  // 0: element ID, 1: side ID, 2: boundary ID
  for (const auto & sbct : fe_sbc_tuples)
  {
    fe_sbnd_elem_ids[std::get<2>(sbct)].insert(std::get<0>(sbct));
    ++n_phantom_elems;
  }

  // STEP 2: generate PD data based on to-be-converted FE mesh and prepare for new mesh

  PeridynamicsMesh & pd_mesh = dynamic_cast<PeridynamicsMesh &>(*_mesh);
  // generate PD node data
  pd_mesh.createPeridynamicsMeshData(
      *old_mesh, conv_elem_ids, _connect_blk_id_pairs, _non_connect_blk_id_pairs);

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
  new_mesh->reserve_nodes(n_pd_nodes + n_fe_nodes);
  new_mesh->reserve_elem(n_pd_bonds + n_fe_elems + n_phantom_elems);

  BoundaryInfo & new_boundary_info = new_mesh->get_boundary_info();

  // STEP 3: add points of PD and FE (retained and/or non-converted) nodes, if any, to new mesh

  // save PD nodes to new mesh first
  unsigned int new_node_id = 0;
  // map of IDs of converted FE elements and PD nodes
  std::map<dof_id_type, dof_id_type> fe_elem_pd_node_map;
  for (const auto & eid : conv_elem_ids)
  {
    new_mesh->add_point(old_mesh->elem_ptr(eid)->centroid(), new_node_id);
    fe_elem_pd_node_map.insert(std::make_pair(eid, new_node_id));

    ++new_node_id;
  }
  // then save both retained and non-converted FE nodes, if any, to the new mesh
  // map of IDs of the same point in old and new meshes
  std::map<dof_id_type, dof_id_type> fe_nodes_map;
  for (const auto & nid : fe_nodes_ids)
  {
    new_mesh->add_point(*old_mesh->node_ptr(nid), new_node_id);
    fe_nodes_map.insert(std::make_pair(old_mesh->node_ptr(nid)->id(), new_node_id));

    ++new_node_id;
  }

  // STEP 4: generate PD, phantom, and FE elems using retained and/or non-converted meshes if any

  // first, generate PD elements for new mesh
  unsigned int new_elem_id = 0;
  for (unsigned int i = 0; i < n_pd_nodes; ++i)
  {
    std::vector<dof_id_type> pd_node_neighbors = pd_mesh.getNeighbors(i); // neighbors of a PD node
    for (unsigned int j = 0; j < pd_node_neighbors.size(); ++j)
      if (pd_node_neighbors[j] > i)
      {
        SubdomainID bid_i = pd_mesh.getNodeBlockID(i);
        SubdomainID bid_j = pd_mesh.getNodeBlockID(pd_node_neighbors[j]);
        Elem * new_elem = new Edge2;
        new_elem->set_id(new_elem_id);
        if (bid_i == bid_j) // assign block ID to PD non-interfacial elems
          if (_single_converted_blk)
            new_elem->subdomain_id() = min_converted_fe_blk_id + 1000;
          else
            new_elem->subdomain_id() = bid_i;
        else if (_single_interface_blk) // assign block ID (max_fe_blk_id + 1 + 1000) to all PD
                                        // interfacial elems
          new_elem->subdomain_id() = max_fe_blk_id + 1 + 1000;
        else // assign a new block ID (node i blk ID + node j blk ID) to this PD interfacial elems
          new_elem->subdomain_id() = bid_i + bid_j;

        new_elem = new_mesh->add_elem(new_elem);
        new_elem->set_node(0) = new_mesh->node_ptr(i);
        new_elem->set_node(1) = new_mesh->node_ptr(pd_node_neighbors[j]);

        ++new_elem_id;
      }
  }

  if (_single_converted_blk) // update PD node block ID
    pd_mesh.setNodeBlockID(min_converted_fe_blk_id + 1000);

  // then generate phantom elements for sidesets in PD mesh, this is optional
  std::map<std::pair<dof_id_type, dof_id_type>, std::set<dof_id_type>> elem_edge_node;
  if (_construct_pd_sideset)
    for (const auto & bidit : fe_sbnd_ids)
      for (const auto & eidit : fe_sbnd_elem_ids[bidit])
      {
        bool should_add = false;
        Elem * old_elem = old_mesh->elem_ptr(eidit);
        if (_conv_blk_ids.count(old_elem->subdomain_id()))
        {
          std::vector<dof_id_type> node_ids;
          if (new_mesh->mesh_dimension() == 2) // 2D
          {
            node_ids.resize(3);
            node_ids[0] = fe_elem_pd_node_map.at(eidit);
            Point p0 = *new_mesh->node_ptr(node_ids[0]);
            // search for two more nodes to construct a phantom 3-node triangular element
            if (old_elem->n_nodes() == 3 ||
                old_elem->n_nodes() == 6) // original triangular FE elements: 3-node or 6-node
            {
              // check existence of counterclockwise nodes ordering
              for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
              {
                Elem * nb = old_elem->neighbor_ptr(i);
                if (nb != nullptr)
                {
                  for (unsigned int j = 0; j < nb->n_neighbors(); ++j)
                  {
                    Elem * nb_nb = nb->neighbor_ptr(j);
                    if (nb_nb != nullptr && fe_sbnd_elem_ids[bidit].count(nb_nb->id()) &&
                        nb_nb->id() != eidit)
                    {
                      Point p1 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(nb_nb->id()));
                      Point p2 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(nb->id()));
                      Real val =
                          (p1(1) - p0(1)) * (p2(0) - p1(0)) - (p1(0) - p0(0)) * (p2(1) - p1(1));
                      if (val < 0) // counterclockwise
                      {
                        node_ids[1] = fe_elem_pd_node_map.at(nb_nb->id());
                        node_ids[2] = fe_elem_pd_node_map.at(nb->id());
                        should_add = true;
                      }
                    }
                  }
                }
              }
            }
            else // original quadrilateral FE elements: 4-node, 8-node and 9-node
            {
              // find potential nodes for construction of the phantom triangular element
              std::vector<dof_id_type> boundary_node_ids;
              for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
              {
                Elem * nb = old_elem->neighbor_ptr(i);
                if (nb != nullptr)
                {
                  if (fe_sbnd_elem_ids[bidit].count(nb->id()))
                    boundary_node_ids.push_back(fe_elem_pd_node_map.at(nb->id()));
                  else
                    node_ids[2] = fe_elem_pd_node_map.at(nb->id());
                }
              }
              // check existence of counterclockwise ordering based on the above found nodes
              Point p2 = *new_mesh->node_ptr(node_ids[2]);
              for (unsigned int i = 0; i < boundary_node_ids.size(); ++i)
              {
                Point p1 = *new_mesh->node_ptr(boundary_node_ids[i]);
                Real val = (p1(1) - p0(1)) * (p2(0) - p1(0)) - (p1(0) - p0(0)) * (p2(1) - p1(1));
                if (val < 0) // counterclockwise
                {
                  node_ids[1] = boundary_node_ids[i];
                  should_add = true;
                }
              }
            }

            if (should_add)
            {
              Elem * new_elem = new Tri3;
              new_elem->set_id(new_elem_id);
              if (_single_converted_blk)
                new_elem->subdomain_id() = min_converted_fe_blk_id + 10000;
              else
                new_elem->subdomain_id() = old_elem->subdomain_id() + 10000;
              new_elem = new_mesh->add_elem(new_elem);
              new_elem->set_node(0) = new_mesh->node_ptr(node_ids[0]);
              new_elem->set_node(1) = new_mesh->node_ptr(node_ids[1]);
              new_elem->set_node(2) = new_mesh->node_ptr(node_ids[2]);

              ++new_elem_id;

              new_boundary_info.add_side(new_elem, 0, 1000 + bidit);
              if (old_boundary_info.get_sideset_name(bidit) != "")
                new_boundary_info.sideset_name(1000 + bidit) =
                    "pd_side_" + old_boundary_info.get_sideset_name(bidit);
            }
          }
          else // 3D
          {
            node_ids.resize(4);
            node_ids[0] = fe_elem_pd_node_map.at(eidit);
            Point p0 = *new_mesh->node_ptr(node_ids[0]);
            // search for three more nodes to construct a phantom 4-node tetrahedral element

            if (old_elem->n_nodes() == 4 ||
                old_elem->n_nodes() == 10) // original tetrahedral FE elements: 4-node or 10-node
            {
              // construct phantom element based on original tet mesh
              mooseError("Peridynamics sideset generation doesn't accept tetrahedral elements!");
            }
            else // original hexahedral FE elements
            {
              std::vector<dof_id_type> boundary_elem_ids;
              for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
              {
                Elem * nb = old_elem->neighbor_ptr(i);
                if (nb != nullptr)
                {
                  if (fe_sbnd_elem_ids[bidit].count(nb->id()))
                    boundary_elem_ids.push_back(nb->id());
                  else
                    node_ids[3] = fe_elem_pd_node_map.at(nb->id());
                }
              }
              // choose three nodes ordered in a way such that the normal points to the fourth node
              Point p3 = *new_mesh->node_ptr(node_ids[3]);
              for (unsigned int i = 0; i < boundary_elem_ids.size(); ++i)
              {
                Elem * nb_i = old_mesh->elem_ptr(boundary_elem_ids[i]);
                for (unsigned int j = i + 1; j < boundary_elem_ids.size(); ++j)
                {
                  should_add = false;
                  Elem * nb_j = old_mesh->elem_ptr(boundary_elem_ids[j]);
                  unsigned int common_nb = 0;
                  for (unsigned int k = 0; k < nb_j->n_neighbors();
                       ++k) // check whether nb_i and nb_j shares two common neighbors
                  {
                    if (nb_j->neighbor_ptr(k) != nullptr &&
                        nb_i->has_neighbor(nb_j->neighbor_ptr(k)))
                      ++common_nb;
                  }
                  if (common_nb == 2)
                  {
                    should_add = true;
                    // check whether this new elem overlaps with already created elems by the saved
                    // edge and nodes of previously created phantom elems
                    std::pair<dof_id_type, dof_id_type> pair;
                    if (eidit < boundary_elem_ids[i])
                      pair = std::make_pair(eidit, boundary_elem_ids[i]);
                    else
                      pair = std::make_pair(boundary_elem_ids[i], eidit);

                    if (elem_edge_node.count(pair))
                    {
                      for (const auto & nbid : elem_edge_node[pair])
                        if (nbid != old_elem->id() && old_mesh->elem_ptr(nbid)->has_neighbor(nb_j))
                          should_add = false;
                    }

                    if (eidit < boundary_elem_ids[j])
                      pair = std::make_pair(eidit, boundary_elem_ids[j]);
                    else
                      pair = std::make_pair(boundary_elem_ids[j], eidit);
                    if (elem_edge_node.count(pair))
                    {
                      for (const auto & nbid : elem_edge_node[pair])
                        if (nbid != old_elem->id() && old_mesh->elem_ptr(nbid)->has_neighbor(nb_i))
                          should_add = false;
                    }

                    if (should_add)
                    {
                      Point p1 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(boundary_elem_ids[i]));
                      Point p2 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(boundary_elem_ids[j]));
                      // check whether the normal of face formed by p0, p1 and p2 points to p3
                      Real val =
                          ((p1(1) - p0(1)) * (p2(2) - p0(2)) - (p1(2) - p0(2)) * (p2(1) - p0(1))) *
                              (p3(0) - p0(0)) +
                          ((p1(2) - p0(2)) * (p2(0) - p0(0)) - (p1(0) - p0(0)) * (p2(2) - p0(2))) *
                              (p3(1) - p0(1)) +
                          ((p1(0) - p0(0)) * (p2(1) - p0(1)) - (p1(1) - p0(1)) * (p2(0) - p0(0))) *
                              (p3(2) - p0(2));
                      if (val > 0) // normal point to p3
                      {
                        node_ids[1] = fe_elem_pd_node_map.at(boundary_elem_ids[i]);
                        node_ids[2] = fe_elem_pd_node_map.at(boundary_elem_ids[j]);
                      }
                      else
                      {
                        node_ids[1] = fe_elem_pd_node_map.at(boundary_elem_ids[j]);
                        node_ids[2] = fe_elem_pd_node_map.at(boundary_elem_ids[i]);
                      }

                      // construct the new phantom element
                      Elem * new_elem = new Tet4;
                      new_elem->set_id(new_elem_id);
                      if (_single_converted_blk)
                        new_elem->subdomain_id() = min_converted_fe_blk_id + 10000;
                      else
                        new_elem->subdomain_id() = old_elem->subdomain_id() + 10000;
                      new_elem = new_mesh->add_elem(new_elem);
                      new_elem->set_node(0) = new_mesh->node_ptr(node_ids[0]);
                      new_elem->set_node(1) = new_mesh->node_ptr(node_ids[1]);
                      new_elem->set_node(2) = new_mesh->node_ptr(node_ids[2]);
                      new_elem->set_node(3) = new_mesh->node_ptr(node_ids[3]);

                      // save the edges and nodes used in the new phantom elem
                      if (eidit < boundary_elem_ids[i])
                        elem_edge_node[std::make_pair(eidit, boundary_elem_ids[i])].insert(
                            boundary_elem_ids[j]);
                      else
                        elem_edge_node[std::make_pair(boundary_elem_ids[i], eidit)].insert(
                            boundary_elem_ids[j]);

                      if (eidit < boundary_elem_ids[j])
                        elem_edge_node[std::make_pair(eidit, boundary_elem_ids[j])].insert(
                            boundary_elem_ids[i]);
                      else
                        elem_edge_node[std::make_pair(boundary_elem_ids[j], eidit)].insert(
                            boundary_elem_ids[i]);

                      ++new_elem_id;

                      new_boundary_info.add_side(new_elem, 0, 1000 + bidit);
                      if (old_boundary_info.get_sideset_name(bidit) != "")
                        new_boundary_info.sideset_name(1000 + bidit) =
                            "pd_side_" + old_boundary_info.get_sideset_name(bidit);
                    }
                  }
                }
              }
            }
          }
        }
      }

  // next, save non-converted or retained FE elements if any to new mesh
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

  // peridynamics ONLY accept nodesets and sidesets
  // nodeset consisting single node in the converted FE block will no longer be available
  if (old_boundary_info.n_edge_conds())
    mooseError("PeridynamicsMesh doesn't support edgesets!");

  // check the existence of nodesets, if exist, build sidesets in case this hasn't been done yet
  if (old_boundary_info.n_nodeset_conds())
    old_boundary_info.build_side_list_from_node_list();

  // first, create a tuple to collect all sidesets (including those converted from nodesets) in the
  // old mesh
  auto old_fe_sbc_tuples = old_boundary_info.build_side_list();
  // 0: element ID, 1: side ID, 2: boundary ID
  // map of set of elem IDs connected to each boundary in the old mesh
  std::map<boundary_id_type, std::set<dof_id_type>> old_fe_bnd_elem_ids;
  // map of set of side ID for each elem in the old mesh
  std::map<dof_id_type, std::map<boundary_id_type, dof_id_type>> old_fe_elem_bnd_side_ids;
  for (const auto & sbct : old_fe_sbc_tuples)
  {
    old_fe_bnd_elem_ids[std::get<2>(sbct)].insert(std::get<0>(sbct));
    old_fe_elem_bnd_side_ids[std::get<0>(sbct)].insert(
        std::make_pair(std::get<2>(sbct), std::get<1>(sbct)));
  }

  // next, convert element lists in old mesh to PD nodesets in new mesh
  std::set<boundary_id_type> old_side_bid(old_boundary_info.get_side_boundary_ids());

  // loop through all old FE _sideset_ boundaries
  for (const auto & sbid : old_side_bid)
    for (const auto & beid : old_fe_bnd_elem_ids[sbid])
      if (conv_elem_ids.count(beid)) // for converted FE mesh
      {
        // save corresponding boundaries on converted FE mesh to PD nodes
        new_boundary_info.add_node(new_mesh->node_ptr(fe_elem_pd_node_map.at(beid)), sbid + 1000);
        if (old_boundary_info.get_sideset_name(sbid) != "")
          new_boundary_info.nodeset_name(sbid + 1000) =
              "pd_nodes_" + old_boundary_info.get_sideset_name(sbid);

        if (_retain_fe_mesh) // if retained, copy the corresponding boundaries, if any, to new mesh
                             // from old mesh
        {
          new_boundary_info.add_side(
              new_mesh->elem_ptr(fe_elems_map[beid]), old_fe_elem_bnd_side_ids[beid][sbid], sbid);
          new_boundary_info.sideset_name(sbid) = old_boundary_info.get_sideset_name(sbid);
        }
      }
      else // for non-converted FE mesh, if any, copy the corresponding boundaries to new mesh
           // from old mesh
      {
        new_boundary_info.add_side(
            new_mesh->elem_ptr(fe_elems_map[beid]), old_fe_elem_bnd_side_ids[beid][sbid], sbid);
        new_boundary_info.sideset_name(sbid) = old_boundary_info.get_sideset_name(sbid);
      }

  // similar for sideset above, save _nodesets_ of non-converted and/or retained FE mesh, if any, to
  // new mesh
  auto old_node_bc_tuples = old_boundary_info.build_node_list();
  // 0: node ID, 1: boundary ID
  std::map<boundary_id_type, std::set<dof_id_type>> old_bnd_node_ids;
  for (const auto & nbct : old_node_bc_tuples)
    old_bnd_node_ids[std::get<1>(nbct)].insert(std::get<0>(nbct));

  std::set<boundary_id_type> old_node_bid(old_boundary_info.get_node_boundary_ids());

  for (const auto & nbid : old_node_bid)
    for (const auto & bnid : old_bnd_node_ids[nbid])
      if (fe_nodes_ids.count(bnid))
      {
        new_boundary_info.add_node(new_mesh->node_ptr(fe_nodes_map.at(bnid)), nbid);
        new_boundary_info.nodeset_name(nbid) = old_boundary_info.get_sideset_name(nbid);
      }

  // create nodesets to include all PD nodes for PD blocks in the new mesh
  for (unsigned int i = 0; i < n_pd_nodes; ++i)
  {
    if (_conv_blk_ids.size() > 1 && !_single_converted_blk)
    {
      unsigned int j = 0;
      for (const auto & blk_id : _conv_blk_ids)
      {
        ++j;
        SubdomainID real_blk_id =
            blk_id + 1000; // account for the 1000 increment after converting to PD mesh
        if (pd_mesh.getNodeBlockID(i) == real_blk_id)
        {
          new_boundary_info.add_node(new_mesh->node_ptr(i), 999 - j);
          new_boundary_info.nodeset_name(999 - j) =
              "pd_nodes_block_" + Moose::stringify(blk_id + 1000);
        }
      }
    }

    new_boundary_info.add_node(new_mesh->node_ptr(i), 999);
    new_boundary_info.nodeset_name(999) = "pd_nodes_all";
  }

  old_mesh.reset(); // destroy the old_mesh unique_ptr

  return dynamic_pointer_cast<MeshBase>(new_mesh);
}
