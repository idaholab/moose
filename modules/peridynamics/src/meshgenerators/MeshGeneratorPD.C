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
  params.addClassDescription("Mesh generator class to convert FE mesh to PD mesh");

  params.addRequiredParam<MeshGeneratorName>("input",
                                             "The mesh based on which PD mesh will be created");
  params.addParam<std::vector<SubdomainID>>("blocks_to_pd",
                                            "IDs of the FE mesh blocks to be converted to PD mesh");
  params.addParam<std::vector<SubdomainID>>(
      "blocks_as_fe",
      "IDs of the FE mesh blocks to not be converted to PD mesh. This should only be used when the "
      "number of to-be-converted FE blocks is considerably large.");
  params.addRequiredParam<bool>(
      "retain_fe_mesh", "Whether to retain the FE mesh or not after conversion into PD mesh");
  params.addParam<bool>(
      "construct_pd_sidesets",
      false,
      "Whether to construct PD sidesets based on the sidesets in original FE mesh");
  params.addParam<std::vector<boundary_id_type>>(
      "sidesets_to_pd", "IDs of the FE sidesets to be reconstructed based on converted PD mesh");
  params.addParam<std::vector<std::vector<SubdomainID>>>(
      "bonding_block_pairs",
      "List of FE block pairs between which inter-block bonds will be created after being "
      "converted into PD mesh");
  params.addParam<std::vector<std::vector<SubdomainID>>>(
      "non_bonding_block_pairs",
      "List of FE block pairs between which inter-block bonds will NOT be created after being "
      "converted into PD mesh");
  params.addParam<bool>(
      "merge_pd_blocks",
      false,
      "Whether to merge all converted PD mesh blocks into a single block. This is "
      "used when all PD blocks have the same properties");
  params.addParam<bool>(
      "merge_pd_interfacial_blocks",
      false,
      "Whether to merge all PD interfacial mesh blocks into a single block. This is used "
      "when all PD interfacial blocks have the same properties");

  return params;
}

MeshGeneratorPD::MeshGeneratorPD(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _has_blks_to_pd(isParamValid("blocks_to_pd")),
    _has_blks_as_fe(isParamValid("blocks_as_fe")),
    _retain_fe_mesh(getParam<bool>("retain_fe_mesh")),
    _merge_pd_blks(getParam<bool>("merge_pd_blocks")),
    _construct_pd_sideset(getParam<bool>("construct_pd_sidesets")),
    _has_sidesets_to_pd(isParamValid("sidesets_to_pd")),
    _has_bonding_blk_pairs(isParamValid("bonding_block_pairs")),
    _has_non_bonding_blk_pairs(isParamValid("non_bonding_block_pairs")),
    _merge_pd_interfacial_blks(getParam<bool>("merge_pd_interfacial_blocks"))
{
  if (_has_blks_to_pd && _has_blks_as_fe)
    mooseError("Please specifiy either 'blocks_to_pd' or 'blocks_as_fe'!");

  if (_has_blks_as_fe)
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("blocks_as_fe");
    for (unsigned int i = 0; i < ids.size(); ++i)
      _blks_as_fe.insert(ids[i]);
  }

  if (_has_blks_to_pd)
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("blocks_to_pd");
    for (unsigned int i = 0; i < ids.size(); ++i)
      _blks_to_pd.insert(ids[i]);
  }

  if (!_construct_pd_sideset && _has_sidesets_to_pd)
    mooseError("'sidesets_to_pd' is provided without setting "
               "'construct_pd_sidesets' to 'true'!");

  if (_has_sidesets_to_pd)
  {
    std::vector<boundary_id_type> ids = getParam<std::vector<boundary_id_type>>("sidesets_to_pd");
    for (unsigned int i = 0; i < ids.size(); ++i)
      _fe_sidesets_for_pd_construction.insert(ids[i]);
  }

  if (_has_bonding_blk_pairs && _has_non_bonding_blk_pairs)
    mooseError("Please specifiy either 'bonding_block_pairs' or "
               "'non_bonding_block_pairs'!");

  _pd_bonding_blk_pairs.clear();
  if (_has_bonding_blk_pairs)
  {
    std::vector<std::vector<SubdomainID>> id_pairs =
        getParam<std::vector<std::vector<SubdomainID>>>("bonding_block_pairs");

    for (unsigned int i = 0; i < id_pairs.size(); ++i)
      _pd_bonding_blk_pairs.insert(std::make_pair(id_pairs[i][0] + _pd_blk_offset_number,
                                                  id_pairs[i][1] + _pd_blk_offset_number));
  } // considered the renumbering of IDs of converted blocks

  _pd_non_bonding_blk_pairs.clear();
  if (_has_non_bonding_blk_pairs)
  {
    std::vector<std::vector<SubdomainID>> id_pairs =
        getParam<std::vector<std::vector<SubdomainID>>>("non_bonding_block_pairs");

    for (unsigned int i = 0; i < id_pairs.size(); ++i)
      _pd_non_bonding_blk_pairs.insert(std::make_pair(id_pairs[i][0] + _pd_blk_offset_number,
                                                      id_pairs[i][1] + _pd_blk_offset_number));
  } // considered the renumbering of IDs of converted blocks
}

std::unique_ptr<MeshBase>
MeshGeneratorPD::generate()
{
  // get the MeshBase object this generator will be working on
  std::unique_ptr<MeshBase> old_mesh = std::move(_input);

  // STEP 1: obtain FE block(s) and elements to be converted to PD mesh

  // get the IDs of all available blocks in the input FE mesh
  std::set<SubdomainID> all_fe_blks;
  for (const auto & old_elem : old_mesh->element_ptr_range())
    all_fe_blks.insert(old_elem->subdomain_id());

  // double check the existence of the block ids provided by user
  if (_has_blks_to_pd)
    for (const auto & blkit : _blks_to_pd)
      if (!all_fe_blks.count(blkit))
        mooseError("Block ID ", blkit, " in the 'blocks_to_pd' does not exist in the FE mesh!");

  if (_has_blks_as_fe)
    for (const auto & blkit : _blks_as_fe)
      if (!all_fe_blks.count(blkit))
        mooseError("Block ID ", blkit, " in the 'blocks_as_fe' does not exist in the FE mesh!");

  // the maximum FE block ID, which will be used in determine the block ID for interfacial bond in
  // the case of single interface block
  const unsigned int max_fe_blk_id = *all_fe_blks.rbegin();

  // categorize mesh blocks into converted and non-converted blocks
  if (_has_blks_to_pd)
    std::set_difference(all_fe_blks.begin(),
                        all_fe_blks.end(),
                        _blks_to_pd.begin(),
                        _blks_to_pd.end(),
                        std::inserter(_blks_as_fe, _blks_as_fe.begin()));
  else if (_has_blks_as_fe)
    std::set_difference(all_fe_blks.begin(),
                        all_fe_blks.end(),
                        _blks_as_fe.begin(),
                        _blks_as_fe.end(),
                        std::inserter(_blks_to_pd, _blks_to_pd.begin()));
  else // if no block ids provided by user, by default, convert all FE mesh to PD mesh
    _blks_to_pd = all_fe_blks;

  if (_has_bonding_blk_pairs)
  {
    for (const auto & blk_pair : _pd_bonding_blk_pairs)
    {
      if (!all_fe_blks.count(blk_pair.first - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.first - _pd_blk_offset_number,
                   " in the 'bonding_block_pairs' does not exist in the FE mesh!");
      if (!all_fe_blks.count(blk_pair.second - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.second - _pd_blk_offset_number,
                   " in the 'bonding_block_pairs' does not exist in the FE mesh!");
      if (!_blks_to_pd.count(blk_pair.first - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.first - _pd_blk_offset_number,
                   " in the 'bonding_block_pairs' is a FE mesh block!");
      if (!_blks_to_pd.count(blk_pair.second - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.second - _pd_blk_offset_number,
                   " in the 'bonding_block_pairs' is a FE mesh block!");
    }
  }

  if (_has_non_bonding_blk_pairs)
    for (const auto & blk_pair : _pd_non_bonding_blk_pairs)
    {
      if (!all_fe_blks.count(blk_pair.first - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.first - _pd_blk_offset_number,
                   " in the 'non_bonding_block_pairs' does not exist in the FE mesh!");
      if (!all_fe_blks.count(blk_pair.second - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.second - _pd_blk_offset_number,
                   " in the 'non_bonding_block_pairs' does not exist in the FE mesh!");
      if (!_blks_as_fe.count(blk_pair.first - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.first - _pd_blk_offset_number,
                   " in the 'non_bonding_block_pairs' is a FE mesh block!");
      if (!_blks_as_fe.count(blk_pair.second - _pd_blk_offset_number))
        mooseError("Block ID ",
                   blk_pair.second - _pd_blk_offset_number,
                   " in the 'non_bonding_block_pairs' is a FE mesh block!");
    }

  // the minimum converted FE block ID, which will be used to assign block ID for non-interfacial
  // bond in the case of combine converted blocks
  const unsigned int min_converted_fe_blk_id = *_blks_to_pd.begin();

  // IDs of to-be-converted FE elems
  std::set<dof_id_type> elems_to_pd;
  // retained FE mesh and non-converted FE mesh, if any
  std::set<dof_id_type> fe_nodes;
  std::set<dof_id_type> fe_elems;
  for (const auto & old_elem : old_mesh->element_ptr_range())
    if (_blks_to_pd.count(old_elem->subdomain_id())) // record to-be-converted FE elem IDs
    {
      elems_to_pd.insert(old_elem->id());
      if (_retain_fe_mesh) // save converted elems and their nodes if need to be retained
      {
        fe_elems.insert(old_elem->id());
        for (unsigned int i = 0; i < old_elem->n_nodes(); ++i)
          fe_nodes.insert(old_elem->node_id(i));
      }
    }
    else // save non-converted elements and their nodes
    {
      fe_elems.insert(old_elem->id());
      for (unsigned int i = 0; i < old_elem->n_nodes(); ++i)
        fe_nodes.insert(old_elem->node_id(i));
    }

  // number of FE elements and nodes in the old mesh to be saved in the new mesh
  dof_id_type n_fe_nodes = fe_nodes.size();
  dof_id_type n_fe_elems = fe_elems.size();
  dof_id_type n_phantom_elems = 0;

  // determine the number of phantom elements to be generated in the new mesh based on sideset in
  // old mesh
  BoundaryInfo & old_boundary_info = old_mesh->get_boundary_info();
  const std::set<boundary_id_type> & all_fe_sidesets = old_boundary_info.get_side_boundary_ids();

  // double check the existence of the sideset ids provided by user
  if (_has_sidesets_to_pd)
  {
    for (const auto & sideset : _fe_sidesets_for_pd_construction)
      if (!all_fe_sidesets.count(sideset))
        mooseError("Sideset ID ",
                   sideset,
                   " in the 'sidesets_to_pd' does not exist in the finite "
                   "element mesh!");
  }
  else // save the IDs of all FE sidesets, this will be updated to the converted FE block later
    _fe_sidesets_for_pd_construction = all_fe_sidesets;

  // determine number of FE side elements, the number of actual phantom elements is less than or
  // equal to the number of FE side elements, this number is used to reserve number of elements
  // in the new mesh only
  std::map<boundary_id_type, std::set<dof_id_type>> fe_bid_eid;
  auto fe_sbc_tuples = old_boundary_info.build_side_list();
  // 0: element ID, 1: side ID, 2: boundary ID
  for (const auto & sbct : fe_sbc_tuples)
    if (_fe_sidesets_for_pd_construction.count(
            std::get<2>(sbct))) // save elements of to-be-constructed sidesets only
    {
      fe_bid_eid[std::get<2>(sbct)].insert(std::get<0>(sbct));
      ++n_phantom_elems;
    }

  // STEP 2: generate PD data based on to-be-converted FE mesh and prepare for new mesh

  PeridynamicsMesh & pd_mesh = dynamic_cast<PeridynamicsMesh &>(*_mesh);
  // generate PD node data, including neighbors
  pd_mesh.createPeridynamicsMeshData(
      *old_mesh, elems_to_pd, _pd_bonding_blk_pairs, _pd_non_bonding_blk_pairs);

  // number of PD elements and nodes to be created
  dof_id_type n_pd_nodes = pd_mesh.nPDNodes();
  dof_id_type n_pd_bonds = pd_mesh.nPDBonds();

  // initialize an empty new mesh object
  std::unique_ptr<MeshBase> new_mesh = buildMeshBaseObject();
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
  for (const auto & eid : elems_to_pd)
  {
    new_mesh->add_point(old_mesh->elem_ptr(eid)->vertex_average(), new_node_id);
    fe_elem_pd_node_map.insert(std::make_pair(eid, new_node_id));

    ++new_node_id;
  }
  // then save both retained and non-converted FE nodes, if any, to the new mesh
  // map of IDs of the same point in old and new meshes
  std::map<dof_id_type, dof_id_type> fe_nodes_map;
  for (const auto & nid : fe_nodes)
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
        if (bid_i == bid_j) // assign block ID to PD non-inter-block bonds
          if (_merge_pd_blks)
            new_elem->subdomain_id() = min_converted_fe_blk_id + _pd_blk_offset_number;
          else
            new_elem->subdomain_id() = bid_i;
        else if (_merge_pd_interfacial_blks) // assign block ID (max_fe_blk_id + 1 +
                                             // _pd_blk_offset_number) to all PD inter-block bonds
          new_elem->subdomain_id() = max_fe_blk_id + 1 + _pd_blk_offset_number;
        else // assign a new block ID (node i blk ID + node j blk ID) to this PD inter-block bonds
          new_elem->subdomain_id() = bid_i + bid_j;

        new_elem = new_mesh->add_elem(new_elem);
        new_elem->set_node(0) = new_mesh->node_ptr(i);
        new_elem->set_node(1) = new_mesh->node_ptr(pd_node_neighbors[j]);

        ++new_elem_id;
      }
  }

  if (_merge_pd_blks) // update PD node block ID if use single block ID for all PD blocks
    pd_mesh.setNodeBlockID(min_converted_fe_blk_id + _pd_blk_offset_number);

  // then generate phantom elements for sidesets in PD mesh, this is optional
  std::map<std::pair<dof_id_type, dof_id_type>, std::set<dof_id_type>> elem_edge_nodes;
  if (_construct_pd_sideset)
  {
    for (const auto & bidit : _fe_sidesets_for_pd_construction)
    {
      for (const auto & eidit : fe_bid_eid[bidit])
      {
        bool should_add = false;
        Elem * old_elem = old_mesh->elem_ptr(eidit);
        if (_blks_to_pd.count(
                old_elem->subdomain_id())) // limit the sidesets to the converted FE blocks only
        {
          std::vector<dof_id_type> pd_nodes;
          Point p0, p1, p2, p3;

          if (old_elem->dim() == 2) // 2D: construct 3-node triangular phanton elems
          {
            pd_nodes.resize(3);
            // the current elem on the boundary side is the first node of a phantom element
            pd_nodes[0] = fe_elem_pd_node_map.at(eidit);
            p0 = *new_mesh->node_ptr(pd_nodes[0]);

            // search for two more nodes to construct a phantom 3-node triangular element
            if (old_elem->n_nodes() == 3 ||
                old_elem->n_nodes() == 6) // original triangular FE elements: 3-node or 6-node
            {
              // one special case of triangular mesh: two elems share a boundary node
              bool has_neighbor_on_boundary = false;
              for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
              {
                Elem * nb = old_elem->neighbor_ptr(i);
                if (nb != nullptr)
                {
                  if (fe_bid_eid[bidit].count(nb->id()))
                  {
                    has_neighbor_on_boundary = true;
                    pd_nodes[1] = fe_elem_pd_node_map.at(nb->id());
                    p1 = *new_mesh->node_ptr(pd_nodes[1]);
                  }
                  else
                  {
                    pd_nodes[2] = fe_elem_pd_node_map.at(nb->id());
                    p2 = *new_mesh->node_ptr(pd_nodes[2]);
                  }
                }
              }

              if (has_neighbor_on_boundary &&
                  (p1(1) - p0(1)) * (p2(0) - p1(0)) - (p1(0) - p0(0)) * (p2(1) - p1(1)) < 0)
              {
                should_add = true;
              }
              else // common case of triangular mesh: three elems share a boundary node
              {
                for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
                {
                  Elem * nb = old_elem->neighbor_ptr(i);

                  if (nb != nullptr)
                  {
                    p2 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(nb->id()));

                    for (unsigned int j = 0; j < nb->n_neighbors(); ++j)
                    {
                      Elem * nb_nb = nb->neighbor_ptr(j);

                      if (nb_nb != nullptr && fe_bid_eid[bidit].count(nb_nb->id()) &&
                          nb_nb->id() != eidit)
                      {
                        p1 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(nb_nb->id()));

                        // add new phantom elem only if (p0, p1, p2) is counterclockwisely ordered
                        if ((p1(1) - p0(1)) * (p2(0) - p1(0)) - (p1(0) - p0(0)) * (p2(1) - p1(1)) <
                            0)
                        {
                          should_add = true;
                          pd_nodes[1] = fe_elem_pd_node_map.at(nb_nb->id());
                          pd_nodes[2] = fe_elem_pd_node_map.at(nb->id());
                        }
                      }
                    }

                    if (!should_add) // if using the neighbor's neighbor is not a success
                    {
                      // one special case of triangular mesh: four elems share a boundary node
                      // other cases of more than four elems share a boundary node is not considered
                      for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
                      {
                        Elem * nb = old_elem->neighbor_ptr(i);

                        if (nb != nullptr)
                        {
                          p2 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(nb->id()));

                          for (unsigned int j = 0; j < nb->n_neighbors(); ++j)
                          {
                            Elem * nb_nb = nb->neighbor_ptr(j);
                            if (nb_nb != nullptr && nb_nb->id() != eidit)
                            {
                              for (unsigned int k = 0; k < nb_nb->n_neighbors(); ++k)
                              {
                                Elem * nb_nb_nb = nb_nb->neighbor_ptr(k);

                                if (nb_nb_nb != nullptr &&
                                    fe_bid_eid[bidit].count(nb_nb_nb->id()) &&
                                    nb_nb_nb->id() != eidit)
                                {
                                  p1 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(nb_nb_nb->id()));

                                  // add new phantom elem only if (p0, p1, p2) is counterclockwisely
                                  // ordered
                                  if ((p1(1) - p0(1)) * (p2(0) - p1(0)) -
                                          (p1(0) - p0(0)) * (p2(1) - p1(1)) <
                                      0)
                                  {
                                    should_add = true;
                                    pd_nodes[1] = fe_elem_pd_node_map.at(nb_nb_nb->id());
                                    pd_nodes[2] = fe_elem_pd_node_map.at(nb->id());
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            else if (old_elem->n_nodes() == 4 || old_elem->n_nodes() == 8 ||
                     old_elem->n_nodes() ==
                         9) // original quadrilateral FE elements: 4-node, 8-node and 9-node
            {
              std::vector<dof_id_type> pd_boundary_node_ids;
              for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
              {
                Elem * nb = old_elem->neighbor_ptr(i);
                if (nb != nullptr && fe_bid_eid[bidit].count(nb->id()))
                  pd_boundary_node_ids.push_back(fe_elem_pd_node_map.at(nb->id()));
              }
              // the neighbor opposite to the boundary side is the third node for the phantom elem
              pd_nodes[2] = fe_elem_pd_node_map.at(
                  old_elem
                      ->neighbor_ptr(old_elem->opposite_side(
                          old_boundary_info.side_with_boundary_id(old_elem, bidit)))
                      ->id());
              p2 = *new_mesh->node_ptr(pd_nodes[2]);

              // if two boundary neighbors, one of them is the second node for the phantom elem
              // if one boundary neighbors, it is the second node for the phantom elem
              for (unsigned int i = 0; i < pd_boundary_node_ids.size(); ++i)
              {
                p1 = *new_mesh->node_ptr(pd_boundary_node_ids[i]);

                // new phantom elem only if (p0, p1, p2) is counterclockwisely orderd
                if ((p1(1) - p0(1)) * (p2(0) - p1(0)) - (p1(0) - p0(0)) * (p2(1) - p1(1)) < 0)
                {
                  should_add = true;
                  pd_nodes[1] = pd_boundary_node_ids[i];
                }
              }
            }
            else
              mooseError("Element type ",
                         old_elem->type(),
                         " is not supported for PD sideset construction!");

            if (should_add)
            {
              Elem * new_elem = new Tri3;
              new_elem->set_id(new_elem_id);
              if (_merge_pd_blks)
                new_elem->subdomain_id() = min_converted_fe_blk_id + _phantom_blk_offset_number;
              else
                new_elem->subdomain_id() = old_elem->subdomain_id() + _phantom_blk_offset_number;
              new_elem = new_mesh->add_elem(new_elem);
              new_elem->set_node(0) = new_mesh->node_ptr(pd_nodes[0]);
              new_elem->set_node(1) = new_mesh->node_ptr(pd_nodes[1]);
              new_elem->set_node(2) = new_mesh->node_ptr(pd_nodes[2]);

              ++new_elem_id;

              new_boundary_info.add_side(new_elem, 0, _pd_blk_offset_number + bidit);
              if (old_boundary_info.get_sideset_name(bidit) != "")
                new_boundary_info.sideset_name(_pd_blk_offset_number + bidit) =
                    "pd_side_" + old_boundary_info.get_sideset_name(bidit);
            }
          }
          else if (old_elem->dim() == 3) // 3D
          {
            pd_nodes.resize(4); // construct 4-node tetrahedral phanton elems
            pd_nodes[0] = fe_elem_pd_node_map.at(eidit);
            p0 = *new_mesh->node_ptr(pd_nodes[0]);
            // search for three more nodes to construct a phantom 4-node tetrahedral element
            if (old_elem->n_nodes() == 4 ||
                old_elem->n_nodes() == 10) // original tetrahedral FE elements: 4-node or 10-node
            {
              // construct phantom element based on original tet mesh
              mooseError("Peridynamics sideset generation does not accept tetrahedral elements!");
            }
            else if (old_elem->n_nodes() == 8 || old_elem->n_nodes() == 20 ||
                     old_elem->n_nodes() ==
                         27) // original hexahedral FE elements: 8-node, 20-node or 27-node
            {
              std::vector<dof_id_type> boundary_elem_ids;
              for (unsigned int i = 0; i < old_elem->n_neighbors(); ++i)
              {
                Elem * nb = old_elem->neighbor_ptr(i);
                if (nb != nullptr && fe_bid_eid[bidit].count(nb->id()))
                  boundary_elem_ids.push_back(nb->id());
              }

              // find the fourth pd node by the boundary side of the element
              pd_nodes[3] = fe_elem_pd_node_map.at(
                  old_elem
                      ->neighbor_ptr(old_elem->opposite_side(
                          old_boundary_info.side_with_boundary_id(old_elem, bidit)))
                      ->id());
              p3 = *new_mesh->node_ptr(pd_nodes[3]);

              // choose two neighbors of current node (total of three pd nodes) ordered in a way
              // such that the normal points to the fourth pd node
              for (unsigned int i = 0; i < boundary_elem_ids.size(); ++i)
              {
                Elem * nb_i = old_mesh->elem_ptr(boundary_elem_ids[i]);
                p1 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(boundary_elem_ids[i]));

                for (unsigned int j = i + 1; j < boundary_elem_ids.size(); ++j)
                {
                  Elem * nb_j = old_mesh->elem_ptr(boundary_elem_ids[j]);
                  p2 = *new_mesh->node_ptr(fe_elem_pd_node_map.at(boundary_elem_ids[j]));

                  if (old_elem->which_neighbor_am_i(nb_i) !=
                      old_elem->opposite_side(old_elem->which_neighbor_am_i(nb_j)))
                  {
                    should_add = true;
                    // check whether this new elem overlaps with already created elems by the
                    // saved edge and nodes of previously created phantom elems
                    std::pair<dof_id_type, dof_id_type> nodes_pair_i;
                    nodes_pair_i.first = std::min(eidit, boundary_elem_ids[i]);
                    nodes_pair_i.second = std::max(eidit, boundary_elem_ids[i]);
                    if (elem_edge_nodes.count(nodes_pair_i))
                    {
                      for (const auto & nbid : elem_edge_nodes[nodes_pair_i])
                        if (nbid != old_elem->id() && old_mesh->elem_ptr(nbid)->has_neighbor(nb_j))
                          should_add = false;
                    }

                    std::pair<dof_id_type, dof_id_type> nodes_pair_j;
                    nodes_pair_j.first = std::min(eidit, boundary_elem_ids[j]);
                    nodes_pair_j.second = std::max(eidit, boundary_elem_ids[j]);
                    if (elem_edge_nodes.count(nodes_pair_j))
                    {
                      for (const auto & nbid : elem_edge_nodes[nodes_pair_j])
                        if (nbid != old_elem->id() && old_mesh->elem_ptr(nbid)->has_neighbor(nb_i))
                          should_add = false;
                    }

                    if (should_add)
                    {
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
                        pd_nodes[1] = fe_elem_pd_node_map.at(boundary_elem_ids[i]);
                        pd_nodes[2] = fe_elem_pd_node_map.at(boundary_elem_ids[j]);
                      }
                      else
                      {
                        pd_nodes[1] = fe_elem_pd_node_map.at(boundary_elem_ids[j]);
                        pd_nodes[2] = fe_elem_pd_node_map.at(boundary_elem_ids[i]);
                      }

                      // construct the new phantom element
                      Elem * new_elem = new Tet4;
                      new_elem->set_id(new_elem_id);

                      if (_merge_pd_blks)
                        new_elem->subdomain_id() =
                            min_converted_fe_blk_id + _phantom_blk_offset_number;
                      else
                        new_elem->subdomain_id() =
                            old_elem->subdomain_id() + _phantom_blk_offset_number;

                      new_elem = new_mesh->add_elem(new_elem);
                      new_elem->set_node(0) = new_mesh->node_ptr(pd_nodes[0]);
                      new_elem->set_node(1) = new_mesh->node_ptr(pd_nodes[1]);
                      new_elem->set_node(2) = new_mesh->node_ptr(pd_nodes[2]);
                      new_elem->set_node(3) = new_mesh->node_ptr(pd_nodes[3]);

                      // save the edges and nodes used in the new phantom elem, which will be used
                      // for creating new phantom elements
                      elem_edge_nodes[nodes_pair_i].insert(boundary_elem_ids[j]);
                      elem_edge_nodes[nodes_pair_j].insert(boundary_elem_ids[i]);

                      ++new_elem_id;

                      new_boundary_info.add_side(new_elem, 0, _pd_blk_offset_number + bidit);
                      if (old_boundary_info.get_sideset_name(bidit) != "")
                      {
                        new_boundary_info.sideset_name(_pd_blk_offset_number + bidit) =
                            "pd_side_" + old_boundary_info.get_sideset_name(bidit);
                      }
                    }
                  }
                }
              }
            }
            else
              mooseError("Element type ",
                         old_elem->type(),
                         " is not supported for PD sidesets construction!");
          }
        }
      }
    }
  }

  // next, save non-converted or retained FE elements, if any, to new mesh
  std::map<dof_id_type, dof_id_type> fe_elems_map; // IDs of the same elem in the old and new meshes
  for (const auto & eid : fe_elems)
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

  // STEP 5: convert old boundary_info to new boundary_info for retained FE mesh

  // peridynamics ONLY accept nodesets and sidesets
  // nodeset consisting single node in the converted FE block will no longer be available
  if (old_boundary_info.n_edge_conds())
    mooseError("PeridynamicsMesh does not support edgesets!");

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
      if (elems_to_pd.count(beid)) // for converted FE mesh
      {
        // save corresponding boundaries on converted FE mesh to PD nodes
        new_boundary_info.add_node(new_mesh->node_ptr(fe_elem_pd_node_map.at(beid)),
                                   sbid + _pd_blk_offset_number);
        if (old_boundary_info.get_sideset_name(sbid) != "")
          new_boundary_info.nodeset_name(sbid + _pd_blk_offset_number) =
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
      if (fe_nodes.count(bnid))
      {
        new_boundary_info.add_node(new_mesh->node_ptr(fe_nodes_map.at(bnid)), nbid);
        new_boundary_info.nodeset_name(nbid) = old_boundary_info.get_sideset_name(nbid);
      }

  // create nodesets to include all PD nodes for PD blocks in the new mesh
  for (unsigned int i = 0; i < n_pd_nodes; ++i)
  {
    if (_blks_to_pd.size() > 1 && !_merge_pd_blks)
    {
      unsigned int j = 0;
      for (const auto & blk_id : _blks_to_pd)
      {
        ++j;
        SubdomainID real_blk_id =
            blk_id + _pd_blk_offset_number; // account for the _pd_blk_offset_number increment after
                                            // converting to PD mesh
        if (pd_mesh.getNodeBlockID(i) == real_blk_id)
        {
          new_boundary_info.add_node(new_mesh->node_ptr(i), _pd_nodeset_offset_number - j);
          new_boundary_info.nodeset_name(_pd_nodeset_offset_number - j) =
              "pd_nodes_block_" + Moose::stringify(blk_id + _pd_blk_offset_number);
        }
      }
    }

    new_boundary_info.add_node(new_mesh->node_ptr(i), _pd_nodeset_offset_number);
    new_boundary_info.nodeset_name(_pd_nodeset_offset_number) = "pd_nodes_all";
  }

  old_mesh.reset(); // destroy the old_mesh unique_ptr

  return dynamic_pointer_cast<MeshBase>(new_mesh);
}
