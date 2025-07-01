//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"
#include "libmesh/id_types.h"
#include "libmesh/int_range.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/utility.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/compare_elems_by_level.h"
#include "libmesh/mesh_communication.h"

#include "timpi/parallel_sync.h"

using namespace libMesh;

namespace MooseMeshUtils
{

void
mergeBoundaryIDsWithSameName(MeshBase & mesh)
{
  // We check if we have the same boundary name with different IDs. If we do, we assign the
  // first ID to every occurrence.
  const auto & side_bd_name_map = mesh.get_boundary_info().get_sideset_name_map();
  const auto & node_bd_name_map = mesh.get_boundary_info().get_nodeset_name_map();
  std::map<boundary_id_type, boundary_id_type> same_name_ids;

  auto populate_map = [](const std::map<boundary_id_type, std::string> & map,
                         std::map<boundary_id_type, boundary_id_type> & same_ids)
  {
    for (const auto & pair_outer : map)
      for (const auto & pair_inner : map)
        // The last condition is needed to make sure we only store one combination
        if (pair_outer.second == pair_inner.second && pair_outer.first != pair_inner.first &&
            same_ids.find(pair_inner.first) == same_ids.end())
          same_ids[pair_outer.first] = pair_inner.first;
  };

  populate_map(side_bd_name_map, same_name_ids);
  populate_map(node_bd_name_map, same_name_ids);

  for (const auto & [id1, id2] : same_name_ids)
    mesh.get_boundary_info().renumber_id(id2, id1);
}

void
changeBoundaryId(MeshBase & mesh,
                 const boundary_id_type old_id,
                 const boundary_id_type new_id,
                 bool delete_prev)
{
  // Get a reference to our BoundaryInfo object, we will use it several times below...
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Container to catch ids passed back from BoundaryInfo
  std::vector<boundary_id_type> old_ids;

  // Only level-0 elements store BCs.  Loop over them.
  for (auto & elem : as_range(mesh.level_elements_begin(0), mesh.level_elements_end(0)))
  {
    unsigned int n_sides = elem->n_sides();
    for (const auto s : make_range(n_sides))
    {
      boundary_info.boundary_ids(elem, s, old_ids);
      if (std::find(old_ids.begin(), old_ids.end(), old_id) != old_ids.end())
      {
        std::vector<boundary_id_type> new_ids(old_ids);
        std::replace(new_ids.begin(), new_ids.end(), old_id, new_id);
        if (delete_prev)
        {
          boundary_info.remove_side(elem, s);
          boundary_info.add_side(elem, s, new_ids);
        }
        else
          boundary_info.add_side(elem, s, new_ids);
      }
    }
  }

  // Remove any remaining references to the old ID from the
  // BoundaryInfo object.  This prevents things like empty sidesets
  // from showing up when printing information, etc.
  if (delete_prev)
    boundary_info.remove_id(old_id);

  // global information may now be out of sync
  mesh.set_isnt_prepared();
}

std::vector<boundary_id_type>
getBoundaryIDs(const MeshBase & mesh,
               const std::vector<BoundaryName> & boundary_name,
               bool generate_unknown)
{
  return getBoundaryIDs(
      mesh, boundary_name, generate_unknown, mesh.get_boundary_info().get_boundary_ids());
}

std::vector<boundary_id_type>
getBoundaryIDs(const MeshBase & mesh,
               const std::vector<BoundaryName> & boundary_name,
               bool generate_unknown,
               const std::set<BoundaryID> & mesh_boundary_ids)
{
  const BoundaryInfo & boundary_info = mesh.get_boundary_info();
  const std::map<BoundaryID, std::string> & sideset_map = boundary_info.get_sideset_name_map();
  const std::map<BoundaryID, std::string> & nodeset_map = boundary_info.get_nodeset_name_map();

  BoundaryID max_boundary_local_id = 0;
  /* It is required to generate a new ID for a given name. It is used often in mesh modifiers such
   * as SideSetsBetweenSubdomains. Then we need to check the current boundary ids since they are
   * changing during "mesh modify()", and figure out the right max boundary ID. Most of mesh
   * modifiers are running in serial, and we won't involve a global communication.
   */
  if (generate_unknown)
  {
    const auto & bids = mesh.is_prepared() ? mesh.get_boundary_info().get_global_boundary_ids()
                                           : mesh.get_boundary_info().get_boundary_ids();
    max_boundary_local_id = bids.empty() ? 0 : *(bids.rbegin());
    /* We should not hit this often */
    if (!mesh.is_prepared() && !mesh.is_serial())
      mesh.comm().max(max_boundary_local_id);
  }

  BoundaryID max_boundary_id = mesh_boundary_ids.empty() ? 0 : *(mesh_boundary_ids.rbegin());

  max_boundary_id =
      max_boundary_id > max_boundary_local_id ? max_boundary_id : max_boundary_local_id;

  std::vector<BoundaryID> ids(boundary_name.size());
  for (const auto i : index_range(boundary_name))
  {
    if (boundary_name[i] == "ANY_BOUNDARY_ID")
    {
      ids.assign(mesh_boundary_ids.begin(), mesh_boundary_ids.end());
      if (i)
        mooseWarning("You passed \"ANY_BOUNDARY_ID\" in addition to other boundary_names.  This "
                     "may be a logic error.");
      break;
    }

    if (boundary_name[i].empty() && !generate_unknown)
      mooseError("Incoming boundary name is empty and we are not generating unknown boundary IDs. "
                 "This is invalid.");

    BoundaryID id;

    if (boundary_name[i].empty() || !MooseUtils::isDigits(boundary_name[i]))
    {
      /**
       * If the conversion from a name to a number fails, that means that this must be a named
       * boundary.  We will look in the complete map for this sideset and create a new name/ID pair
       * if requested.
       */
      if (generate_unknown &&
          !MooseUtils::doesMapContainValue(sideset_map, std::string(boundary_name[i])) &&
          !MooseUtils::doesMapContainValue(nodeset_map, std::string(boundary_name[i])))
        id = ++max_boundary_id;
      else
        id = boundary_info.get_id_by_name(boundary_name[i]);
    }
    else
      id = getIDFromName<BoundaryName, BoundaryID>(boundary_name[i]);

    ids[i] = id;
  }

  return ids;
}

std::set<BoundaryID>
getBoundaryIDSet(const MeshBase & mesh,
                 const std::vector<BoundaryName> & boundary_name,
                 bool generate_unknown)
{
  auto boundaries = getBoundaryIDs(mesh, boundary_name, generate_unknown);
  return std::set<BoundaryID>(boundaries.begin(), boundaries.end());
}

std::vector<subdomain_id_type>
getSubdomainIDs(const MeshBase & mesh, const std::vector<SubdomainName> & subdomain_names)
{
  std::vector<subdomain_id_type> ids;

  // shortcut for "ANY_BLOCK_ID"
  if (subdomain_names.size() == 1 && subdomain_names[0] == "ANY_BLOCK_ID")
  {
    // since get_mesh_subdomains() requires a prepared mesh, we need to check that here
    mooseAssert(mesh.is_prepared(),
                "getSubdomainIDs() should only be called on a prepared mesh if ANY_BLOCK_ID is "
                "used to query all block IDs");
    ids.assign(mesh.get_mesh_subdomains().begin(), mesh.get_mesh_subdomains().end());
    return ids;
  }

  // loop through subdomain names and get IDs (this preserves the order of subdomain_names)
  ids.resize(subdomain_names.size());
  for (auto i : index_range(subdomain_names))
  {
    if (subdomain_names[i] == "ANY_BLOCK_ID")
      mooseError("getSubdomainIDs() accepts \"ANY_BLOCK_ID\" if and only if it is the only "
                 "subdomain name being queried.");
    ids[i] = MooseMeshUtils::getSubdomainID(subdomain_names[i], mesh);
  }

  return ids;
}

std::set<subdomain_id_type>
getSubdomainIDs(const MeshBase & mesh, const std::set<SubdomainName> & subdomain_names)
{
  const auto blk_ids = getSubdomainIDs(
      mesh, std::vector<SubdomainName>(subdomain_names.begin(), subdomain_names.end()));
  return {blk_ids.begin(), blk_ids.end()};
}

BoundaryID
getBoundaryID(const BoundaryName & boundary_name, const MeshBase & mesh)
{
  BoundaryID id = Moose::INVALID_BOUNDARY_ID;
  if (boundary_name.empty())
    return id;

  if (!MooseUtils::isDigits(boundary_name))
    id = mesh.get_boundary_info().get_id_by_name(boundary_name);
  else
    id = getIDFromName<BoundaryName, BoundaryID>(boundary_name);

  return id;
}

SubdomainID
getSubdomainID(const SubdomainName & subdomain_name, const MeshBase & mesh)
{
  if (subdomain_name == "ANY_BLOCK_ID")
    mooseError("getSubdomainID() does not work with \"ANY_BLOCK_ID\"");

  SubdomainID id = Moose::INVALID_BLOCK_ID;
  if (subdomain_name.empty())
    return id;

  if (!MooseUtils::isDigits(subdomain_name))
    id = mesh.get_id_by_name(subdomain_name);
  else
    id = getIDFromName<SubdomainName, SubdomainID>(subdomain_name);

  return id;
}

void
changeSubdomainId(MeshBase & mesh, const subdomain_id_type old_id, const subdomain_id_type new_id)
{
  for (const auto & elem : mesh.element_ptr_range())
    if (elem->subdomain_id() == old_id)
      elem->subdomain_id() = new_id;

  // global cached information may now be out of sync
  mesh.set_isnt_prepared();
}

Point
meshCentroidCalculator(const MeshBase & mesh)
{
  Point centroid_pt = Point(0.0, 0.0, 0.0);
  Real vol_tmp = 0.0;
  for (const auto & elem :
       as_range(mesh.active_local_elements_begin(), mesh.active_local_elements_end()))
  {
    Real elem_vol = elem->volume();
    centroid_pt += (elem->true_centroid()) * elem_vol;
    vol_tmp += elem_vol;
  }
  mesh.comm().sum(centroid_pt);
  mesh.comm().sum(vol_tmp);
  centroid_pt /= vol_tmp;
  return centroid_pt;
}

std::unordered_map<dof_id_type, dof_id_type>
getExtraIDUniqueCombinationMap(const MeshBase & mesh,
                               const std::set<SubdomainID> & block_ids,
                               std::vector<ExtraElementIDName> extra_ids)
{
  // check block restriction
  const bool block_restricted = !block_ids.empty();
  // get element id name of interest in recursive parsing algorithm
  ExtraElementIDName id_name = extra_ids.back();
  extra_ids.pop_back();
  const auto id_index = mesh.get_elem_integer_index(id_name);

  // create base parsed id set
  if (extra_ids.empty())
  {
    // get set of extra id values;
    std::vector<dof_id_type> ids;
    {
      std::set<dof_id_type> ids_set;
      for (const auto & elem : mesh.active_element_ptr_range())
      {
        if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
          continue;
        const auto id = elem->get_extra_integer(id_index);
        ids_set.insert(id);
      }
      mesh.comm().set_union(ids_set);
      ids.assign(ids_set.begin(), ids_set.end());
    }

    // determine new extra id values;
    std::unordered_map<dof_id_type, dof_id_type> parsed_ids;
    for (auto & elem : mesh.active_element_ptr_range())
    {
      if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
        continue;
      parsed_ids[elem->id()] = std::distance(
          ids.begin(), std::lower_bound(ids.begin(), ids.end(), elem->get_extra_integer(id_index)));
    }
    return parsed_ids;
  }

  // if extra_ids is not empty, recursively call getExtraIDUniqueCombinationMap
  const auto base_parsed_ids =
      MooseMeshUtils::getExtraIDUniqueCombinationMap(mesh, block_ids, extra_ids);
  // parsing extra ids based on ref_parsed_ids
  std::vector<std::pair<dof_id_type, dof_id_type>> unique_ids;
  {
    std::set<std::pair<dof_id_type, dof_id_type>> unique_ids_set;
    for (const auto & elem : mesh.active_element_ptr_range())
    {
      if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
        continue;
      const dof_id_type id1 = libmesh_map_find(base_parsed_ids, elem->id());
      const dof_id_type id2 = elem->get_extra_integer(id_index);
      const std::pair<dof_id_type, dof_id_type> ids = std::make_pair(id1, id2);
      unique_ids_set.insert(ids);
    }
    mesh.comm().set_union(unique_ids_set);
    unique_ids.assign(unique_ids_set.begin(), unique_ids_set.end());
  }

  std::unordered_map<dof_id_type, dof_id_type> parsed_ids;

  for (const auto & elem : mesh.active_element_ptr_range())
  {
    if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
      continue;
    const dof_id_type id1 = libmesh_map_find(base_parsed_ids, elem->id());
    const dof_id_type id2 = elem->get_extra_integer(id_index);
    const dof_id_type new_id = std::distance(
        unique_ids.begin(),
        std::lower_bound(unique_ids.begin(), unique_ids.end(), std::make_pair(id1, id2)));
    parsed_ids[elem->id()] = new_id;
  }

  return parsed_ids;
}

bool
isCoPlanar(const std::vector<Point> vec_pts, const Point plane_nvec, const Point fixed_pt)
{
  for (const auto & pt : vec_pts)
    if (!MooseUtils::absoluteFuzzyEqual((pt - fixed_pt) * plane_nvec, 0.0))
      return false;
  return true;
}

bool
isCoPlanar(const std::vector<Point> vec_pts, const Point plane_nvec)
{
  return isCoPlanar(vec_pts, plane_nvec, vec_pts.front());
}

bool
isCoPlanar(const std::vector<Point> vec_pts)
{
  // Assuming that overlapped Points are allowed, the Points that are overlapped with vec_pts[0] are
  // removed before further calculation.
  std::vector<Point> vec_pts_nonzero{vec_pts[0]};
  for (const auto i : index_range(vec_pts))
    if (!MooseUtils::absoluteFuzzyEqual((vec_pts[i] - vec_pts[0]).norm(), 0.0))
      vec_pts_nonzero.push_back(vec_pts[i]);
  // 3 or fewer points are always coplanar
  if (vec_pts_nonzero.size() <= 3)
    return true;
  else
  {
    for (const auto i : make_range(vec_pts_nonzero.size() - 1))
    {
      const Point tmp_pt = (vec_pts_nonzero[i] - vec_pts_nonzero[0])
                               .cross(vec_pts_nonzero[i + 1] - vec_pts_nonzero[0]);
      // if the three points are not collinear, use cross product as the normal vector of the plane
      if (!MooseUtils::absoluteFuzzyEqual(tmp_pt.norm(), 0.0))
        return isCoPlanar(vec_pts_nonzero, tmp_pt.unit());
    }
  }
  // If all the points are collinear, they are also coplanar
  return true;
}

SubdomainID
getNextFreeSubdomainID(MeshBase & input_mesh)
{
  // Call this to get most up to date block id information
  input_mesh.cache_elem_data();

  std::set<SubdomainID> preexisting_subdomain_ids;
  input_mesh.subdomain_ids(preexisting_subdomain_ids);
  if (preexisting_subdomain_ids.empty())
    return 0;
  else
  {
    const auto highest_subdomain_id =
        *std::max_element(preexisting_subdomain_ids.begin(), preexisting_subdomain_ids.end());
    mooseAssert(highest_subdomain_id < std::numeric_limits<SubdomainID>::max(),
                "A SubdomainID with max possible value was found");
    return highest_subdomain_id + 1;
  }
}

BoundaryID
getNextFreeBoundaryID(MeshBase & input_mesh)
{
  auto boundary_ids = input_mesh.get_boundary_info().get_boundary_ids();
  if (boundary_ids.empty())
    return 0;
  return (*boundary_ids.rbegin() + 1);
}

bool
hasSubdomainID(const MeshBase & input_mesh, const SubdomainID & id)
{
  std::set<SubdomainID> mesh_blocks;
  input_mesh.subdomain_ids(mesh_blocks);

  // On a distributed mesh we may have sideset IDs that only exist on
  // other processors
  if (!input_mesh.is_replicated())
    input_mesh.comm().set_union(mesh_blocks);

  return mesh_blocks.count(id) && (id != Moose::INVALID_BLOCK_ID);
}

bool
hasSubdomainName(const MeshBase & input_mesh, const SubdomainName & name)
{
  const auto id = getSubdomainID(name, input_mesh);
  return hasSubdomainID(input_mesh, id);
}

bool
hasBoundaryID(const MeshBase & input_mesh, const BoundaryID id)
{
  const BoundaryInfo & boundary_info = input_mesh.get_boundary_info();
  std::set<boundary_id_type> boundary_ids = boundary_info.get_boundary_ids();

  // On a distributed mesh we may have boundary IDs that only exist on
  // other processors
  if (!input_mesh.is_replicated())
    input_mesh.comm().set_union(boundary_ids);

  return boundary_ids.count(id) && (id != Moose::INVALID_BOUNDARY_ID);
}

bool
hasBoundaryName(const MeshBase & input_mesh, const BoundaryName & name)
{
  const auto id = getBoundaryID(name, input_mesh);
  return hasBoundaryID(input_mesh, id);
}

void
makeOrderedNodeList(std::vector<std::pair<dof_id_type, dof_id_type>> & node_assm,
                    std::vector<dof_id_type> & elem_id_list,
                    std::vector<dof_id_type> & midpoint_node_list,
                    std::vector<dof_id_type> & ordered_node_list,
                    std::vector<dof_id_type> & ordered_elem_id_list)
{
  // a flag to indicate if the ordered_node_list has been reversed
  bool is_flipped = false;
  // Start from the first element, try to find a chain of nodes
  mooseAssert(node_assm.size(), "Node list must not be empty");
  ordered_node_list.push_back(node_assm.front().first);
  if (midpoint_node_list.front() != DofObject::invalid_id)
    ordered_node_list.push_back(midpoint_node_list.front());
  ordered_node_list.push_back(node_assm.front().second);
  ordered_elem_id_list.push_back(elem_id_list.front());
  // Remove the element that has just been added to ordered_node_list
  node_assm.erase(node_assm.begin());
  midpoint_node_list.erase(midpoint_node_list.begin());
  elem_id_list.erase(elem_id_list.begin());
  const unsigned int node_assm_size_0 = node_assm.size();
  for (unsigned int i = 0; i < node_assm_size_0; i++)
  {
    // Find nodes to expand the chain
    dof_id_type end_node_id = ordered_node_list.back();
    auto isMatch1 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.first == end_node_id; };
    auto isMatch2 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.second == end_node_id; };
    auto result = std::find_if(node_assm.begin(), node_assm.end(), isMatch1);
    bool match_first;
    if (result == node_assm.end())
    {
      match_first = false;
      result = std::find_if(node_assm.begin(), node_assm.end(), isMatch2);
    }
    else
    {
      match_first = true;
    }
    // If found, add the node to boundary_ordered_node_list
    if (result != node_assm.end())
    {
      const auto elem_index = std::distance(node_assm.begin(), result);
      if (midpoint_node_list[elem_index] != DofObject::invalid_id)
        ordered_node_list.push_back(midpoint_node_list[elem_index]);
      ordered_node_list.push_back(match_first ? (*result).second : (*result).first);
      node_assm.erase(result);
      midpoint_node_list.erase(midpoint_node_list.begin() + elem_index);
      ordered_elem_id_list.push_back(elem_id_list[elem_index]);
      elem_id_list.erase(elem_id_list.begin() + elem_index);
    }
    // If there are still elements in node_assm and result ==
    // node_assm.end(), this means the curve is not a loop, the
    // ordered_node_list is flipped and try the other direction that has not
    // been examined yet.
    else
    {
      if (is_flipped)
        // Flipped twice; this means the node list has at least two segments.
        throw MooseException("The node list provided has more than one segments.");

      // mark the first flip event.
      is_flipped = true;
      std::reverse(ordered_node_list.begin(), ordered_node_list.end());
      std::reverse(midpoint_node_list.begin(), midpoint_node_list.end());
      std::reverse(ordered_elem_id_list.begin(), ordered_elem_id_list.end());
      // As this iteration is wasted, set the iterator backward
      i--;
    }
  }
}

void
makeOrderedNodeList(std::vector<std::pair<dof_id_type, dof_id_type>> & node_assm,
                    std::vector<dof_id_type> & elem_id_list,
                    std::vector<dof_id_type> & ordered_node_list,
                    std::vector<dof_id_type> & ordered_elem_id_list)
{
  std::vector<dof_id_type> dummy_midpoint_node_list(node_assm.size(), DofObject::invalid_id);
  makeOrderedNodeList(
      node_assm, elem_id_list, dummy_midpoint_node_list, ordered_node_list, ordered_elem_id_list);
}

void
swapNodesInElem(Elem & elem, const unsigned int nd1, const unsigned int nd2)
{
  Node * n_temp = elem.node_ptr(nd1);
  elem.set_node(nd1, elem.node_ptr(nd2));
  elem.set_node(nd2, n_temp);
}

void
extraElemIntegerSwapParametersProcessor(
    const std::string & class_name,
    const unsigned int num_sections,
    const unsigned int num_integers,
    const std::vector<std::vector<std::vector<dof_id_type>>> & elem_integers_swaps,
    std::vector<std::unordered_map<dof_id_type, dof_id_type>> & elem_integers_swap_pairs)
{
  elem_integers_swap_pairs.reserve(num_sections * num_integers);
  for (const auto i : make_range(num_integers))
  {
    const auto & elem_integer_swaps = elem_integers_swaps[i];
    std::vector<std::unordered_map<dof_id_type, dof_id_type>> elem_integer_swap_pairs;
    try
    {
      MooseMeshUtils::idSwapParametersProcessor(class_name,
                                                "elem_integers_swaps",
                                                elem_integer_swaps,
                                                elem_integer_swap_pairs,
                                                i * num_sections);
    }
    catch (const MooseException & e)
    {
      throw MooseException(e.what());
    }

    elem_integers_swap_pairs.insert(elem_integers_swap_pairs.end(),
                                    elem_integer_swap_pairs.begin(),
                                    elem_integer_swap_pairs.end());
  }
}

std::unique_ptr<ReplicatedMesh>
buildBoundaryMesh(const ReplicatedMesh & input_mesh, const boundary_id_type boundary_id)
{
  auto poly_mesh = std::make_unique<ReplicatedMesh>(input_mesh.comm());

  auto side_list = input_mesh.get_boundary_info().build_side_list();

  std::unordered_map<dof_id_type, dof_id_type> old_new_node_map;
  for (const auto & bside : side_list)
  {
    if (std::get<2>(bside) != boundary_id)
      continue;

    const Elem * elem = input_mesh.elem_ptr(std::get<0>(bside));
    const auto side = std::get<1>(bside);
    auto side_elem = elem->build_side_ptr(side);
    auto copy = side_elem->build(side_elem->type());

    for (const auto i : side_elem->node_index_range())
    {
      auto & n = side_elem->node_ref(i);

      if (old_new_node_map.count(n.id()))
        copy->set_node(i, poly_mesh->node_ptr(old_new_node_map[n.id()]));
      else
      {
        Node * node = poly_mesh->add_point(side_elem->point(i));
        copy->set_node(i, node);
        old_new_node_map[n.id()] = node->id();
      }
    }
    poly_mesh->add_elem(copy.release());
  }
  poly_mesh->skip_partitioning(true);
  poly_mesh->prepare_for_use();
  if (poly_mesh->n_elem() == 0)
    mooseError("The input mesh does not have a boundary with id ", boundary_id);

  return poly_mesh;
}

void
createSubdomainFromSidesets(std::unique_ptr<MeshBase> & mesh,
                            std::vector<BoundaryName> boundary_names,
                            const SubdomainID new_subdomain_id,
                            const SubdomainName new_subdomain_name,
                            const std::string type_name)
{
  // Generate a new block id if one isn't supplied.
  SubdomainID new_block_id = new_subdomain_id;

  // Make sure our boundary info and parallel counts are setup
  if (!mesh->is_prepared())
  {
    const bool allow_remote_element_removal = mesh->allow_remote_element_removal();
    // We want all of our boundary elements available, so avoid removing them if they haven't
    // already been so
    mesh->allow_remote_element_removal(false);
    mesh->prepare_for_use();
    mesh->allow_remote_element_removal(allow_remote_element_removal);
  }

  // Check that the sidesets are present in the mesh
  for (const auto & sideset : boundary_names)
    if (!MooseMeshUtils::hasBoundaryName(*mesh, sideset))
      mooseException("The sideset '", sideset, "' was not found within the mesh");

  auto sideset_ids = MooseMeshUtils::getBoundaryIDs(*mesh, boundary_names, true);
  std::set<boundary_id_type> sidesets(sideset_ids.begin(), sideset_ids.end());
  auto side_list = mesh->get_boundary_info().build_side_list();
  if (!mesh->is_serial() && mesh->comm().size() > 1)
  {
    std::vector<Elem *> elements_to_send;
    unsigned short i_need_boundary_elems = 0;
    for (const auto & [elem_id, side, bc_id] : side_list)
    {
      libmesh_ignore(side);
      if (sidesets.count(bc_id))
      {
        // Whether we have this boundary information through our locally owned element or a ghosted
        // element, we'll need the boundary elements for parallel consistent addition
        i_need_boundary_elems = 1;
        auto * elem = mesh->elem_ptr(elem_id);
        if (elem->processor_id() == mesh->processor_id())
          elements_to_send.push_back(elem);
      }
    }

    std::set<const Elem *, libMesh::CompareElemIdsByLevel> connected_elements(
        elements_to_send.begin(), elements_to_send.end());
    std::set<const Node *> connected_nodes;
    reconnect_nodes(connected_elements, connected_nodes);
    std::set<dof_id_type> connected_node_ids;
    for (auto * nd : connected_nodes)
      connected_node_ids.insert(nd->id());

    std::vector<unsigned short> need_boundary_elems(mesh->comm().size());
    mesh->comm().allgather(i_need_boundary_elems, need_boundary_elems);
    std::unordered_map<processor_id_type, decltype(elements_to_send)> push_element_data;
    std::unordered_map<processor_id_type, decltype(connected_nodes)> push_node_data;

    for (const auto pid : index_range(mesh->comm()))
      // Don't need to send to self
      if (pid != mesh->processor_id() && need_boundary_elems[pid])
      {
        if (elements_to_send.size())
          push_element_data[pid] = elements_to_send;
        if (connected_nodes.size())
          push_node_data[pid] = connected_nodes;
      }

    auto node_action_functor = [](processor_id_type, const auto &)
    {
      // Node packing specialization already has unpacked node into mesh, so nothing to do
    };
    Parallel::push_parallel_packed_range(
        mesh->comm(), push_node_data, mesh.get(), node_action_functor);
    auto elem_action_functor = [](processor_id_type, const auto &)
    {
      // Elem packing specialization already has unpacked elem into mesh, so nothing to do
    };
    TIMPI::push_parallel_packed_range(
        mesh->comm(), push_element_data, mesh.get(), elem_action_functor);

    // now that we've gathered everything, we need to rebuild the side list
    side_list = mesh->get_boundary_info().build_side_list();
  }

  std::vector<std::pair<dof_id_type, ElemSideDouble>> element_sides_on_boundary;
  dof_id_type counter = 0;
  for (const auto & triple : side_list)
    if (sidesets.count(std::get<2>(triple)))
    {
      if (auto elem = mesh->query_elem_ptr(std::get<0>(triple)))
      {
        if (!elem->active())
          mooseError(
              "Only active, level 0 elements can be made interior parents of new level 0 lower-d "
              "elements. Make sure that ",
              type_name,
              "s are run before any refinement generators");
        element_sides_on_boundary.push_back(
            std::make_pair(counter, ElemSideDouble(elem, std::get<1>(triple))));
      }
      ++counter;
    }

  dof_id_type max_elem_id = mesh->max_elem_id();
  unique_id_type max_unique_id = mesh->parallel_max_unique_id();

  // Making an important assumption that at least our boundary elements are the same on all
  // processes even in distributed mesh mode (this is reliant on the correct ghosting functors
  // existing on the mesh)
  for (auto & [i, elem_side] : element_sides_on_boundary)
  {
    Elem * elem = elem_side.elem;

    const auto side = elem_side.side;

    // Build a non-proxy element from this side.
    std::unique_ptr<Elem> side_elem(elem->build_side_ptr(side));

    // The side will be added with the same processor id as the parent.
    side_elem->processor_id() = elem->processor_id();

    // Add subdomain ID
    side_elem->subdomain_id() = new_block_id;

    // Also assign the side's interior parent, so it is always
    // easy to figure out the Elem we came from.
    side_elem->set_interior_parent(elem);

    // Add id
    side_elem->set_id(max_elem_id + i);
    side_elem->set_unique_id(max_unique_id + i);

    // Finally, add the lower-dimensional element to the mesh->
    mesh->add_elem(side_elem.release());
  };

  // Assign block name, if provided
  if (new_subdomain_name.size())
    mesh->subdomain_name(new_block_id) = new_subdomain_name;

  const bool skip_partitioning_old = mesh->skip_partitioning();
  mesh->skip_partitioning(true);
  mesh->prepare_for_use();
  mesh->skip_partitioning(skip_partitioning_old);
}

void
convertBlockToMesh(std::unique_ptr<MeshBase> & source_mesh,
                   std::unique_ptr<MeshBase> & target_mesh,
                   const std::vector<SubdomainName> & target_blocks)
{
  if (!source_mesh->is_replicated())
    mooseError("This generator does not support distributed meshes.");

  const auto target_block_ids = MooseMeshUtils::getSubdomainIDs(*source_mesh, target_blocks);

  // Check that the block ids/names exist in the mesh
  std::set<SubdomainID> mesh_blocks;
  source_mesh->subdomain_ids(mesh_blocks);

  for (const auto i : index_range(target_block_ids))
    if (target_block_ids[i] == Moose::INVALID_BLOCK_ID || !mesh_blocks.count(target_block_ids[i]))
    {
      mooseException("The target_block '", target_blocks[i], "' was not found within the mesh.");
    }

  // know which nodes have already been inserted, by tracking the old mesh's node's ids'
  std::unordered_map<dof_id_type, dof_id_type> old_new_node_map;

  for (const auto target_block_id : target_block_ids)
  {

    for (auto elem : source_mesh->active_subdomain_elements_ptr_range(target_block_id))
    {
      if (elem->level() != 0)
        mooseError("Refined blocks are not supported by this generator. "
                   "Can you re-organize mesh generators to refine after converting the block?");

      // make a deep copy so that mutiple meshes' destructors don't segfault at program termination
      auto copy = elem->build(elem->type());

      // index of node in the copy element must be managed manually as there is no intelligent
      // insert method
      dof_id_type copy_n_index = 0;

      // correctly assign new copies of nodes, loop over nodes
      for (dof_id_type i : elem->node_index_range())
      {
        auto & n = elem->node_ref(i);

        if (old_new_node_map.count(n.id()))
        {
          // case where we have already inserted this particular point before
          // then we need to find the already-inserted one and hook it up right
          // to it's respective element
          copy->set_node(copy_n_index++, target_mesh->node_ptr(old_new_node_map[n.id()]));
        }
        else
        {
          // case where we've NEVER inserted this particular point before
          // add them both to the element and the mesh

          // Nodes' IDs are their indexes in the nodes' respective mesh
          // If we set them as invalid they are automatically assigned
          // Add to mesh, auto-assigning a new id.
          Node * node = target_mesh->add_point(elem->point(i));

          // Add to element copy (manually)
          copy->set_node(copy_n_index++, node);

          // remember the (old) ID
          old_new_node_map[n.id()] = node->id();
        }
      }

      // it is ok to release the copy element into the mesh because derived meshes class
      // (ReplicatedMesh, DistributedMesh) manage their own elements, will delete them
      target_mesh->add_elem(copy.release());
    }
  }
}
}
