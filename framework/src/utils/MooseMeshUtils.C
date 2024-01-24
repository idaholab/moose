//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "libmesh/mesh_base.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/utility.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/face_tri3.h"

using namespace libMesh;

namespace MooseMeshUtils
{
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
    for (unsigned int s = 0; s != n_sides; ++s)
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
  for (unsigned int i = 0; i < boundary_name.size(); i++)
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
    {
      std::istringstream ss(boundary_name[i]);
      ss >> id;
      if (ss.fail())
        mooseError("Failed to convert integer ",
                   boundary_name[i],
                   " to a boundary id.  Got ",
                   id,
                   " instead.  Is the integer too large for boundary_id_type?");
    }

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
getSubdomainIDs(const MeshBase & mesh, const std::vector<SubdomainName> & subdomain_name)
{
  std::vector<SubdomainID> ids(subdomain_name.size());

  for (unsigned int i = 0; i < subdomain_name.size(); i++)
    ids[i] = MooseMeshUtils::getSubdomainID(subdomain_name[i], mesh);

  return ids;
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
  {
    std::istringstream ss(boundary_name);
    ss >> id;
  }

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
  {
    std::istringstream ss(subdomain_name);
    ss >> id;
  }

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
  for (unsigned int i = 1; i < vec_pts.size(); i++)
    if (!MooseUtils::absoluteFuzzyEqual((vec_pts[i] - vec_pts[0]).norm(), 0.0))
      vec_pts_nonzero.push_back(vec_pts[i]);
  // 3 or fewer points are always coplanar
  if (vec_pts_nonzero.size() <= 3)
    return true;
  else
  {
    for (unsigned int i = 1; i < vec_pts_nonzero.size() - 1; i++)
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
hasSubdomainID(MeshBase & input_mesh, const SubdomainID & id)
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
hasSubdomainName(MeshBase & input_mesh, const SubdomainName & name)
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
                    std::vector<dof_id_type> & ordered_node_list,
                    std::vector<dof_id_type> & ordered_elem_id_list)
{
  // a flag to indicate if the ordered_node_list has been reversed
  bool isFlipped = false;
  // Start from the first element, try to find a chain of nodes
  mooseAssert(node_assm.size(), "Node list must not be empty");
  ordered_node_list.push_back(node_assm.front().first);
  ordered_node_list.push_back(node_assm.front().second);
  ordered_elem_id_list.push_back(elem_id_list.front());
  // Remove the element that has just been added to ordered_node_list
  node_assm.erase(node_assm.begin());
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
      ordered_node_list.push_back(match_first ? (*result).second : (*result).first);
      node_assm.erase(result);
      const auto elem_index = std::distance(node_assm.begin(), result);
      ordered_elem_id_list.push_back(elem_id_list[elem_index]);
      elem_id_list.erase(elem_id_list.begin() + elem_index);
    }
    // If there are still elements in node_assm and result ==
    // node_assm.end(), this means the curve is not a loop, the
    // ordered_node_list is flipped and try the other direction that has not
    // been examined yet.
    else
    {
      if (isFlipped)
        // Flipped twice; this means the node list has at least two segments.
        throw MooseException("The node list provided has more than one segments.");

      // mark the first flip event.
      isFlipped = true;
      std::reverse(ordered_node_list.begin(), ordered_node_list.end());
      std::reverse(ordered_elem_id_list.begin(), ordered_elem_id_list.end());
      // As this iteration is wasted, set the iterator backward
      i--;
    }
  }
}

void
hexElemSplitter(ReplicatedMesh & mesh,
                const dof_id_type elem_id,
                std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(6);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[std::get<1>(bdry_side_list[i])].push_back(std::get<2>(bdry_side_list[i]));
    }
  }
  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<std::vector<unsigned int>> opt_option;
  std::vector<Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                        mesh.elem_ptr(elem_id)->node_ptr(1),
                                        mesh.elem_ptr(elem_id)->node_ptr(2),
                                        mesh.elem_ptr(elem_id)->node_ptr(3),
                                        mesh.elem_ptr(elem_id)->node_ptr(4),
                                        mesh.elem_ptr(elem_id)->node_ptr(5),
                                        mesh.elem_ptr(elem_id)->node_ptr(6),
                                        mesh.elem_ptr(elem_id)->node_ptr(7)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  auto optimized_node_list = hexNodeOptimizer(elem_node_list, rotated_tet_face_indices);

  std::vector<Elem *> elems_Tet4;
  for (unsigned int i = 0; i < optimized_node_list.size(); i++)
  {
    elems_Tet4.push_back(mesh.add_elem(new Tet4));
    elems_Tet4.back()->set_node(0) = optimized_node_list[i][0];
    elems_Tet4.back()->set_node(1) = optimized_node_list[i][1];
    elems_Tet4.back()->set_node(2) = optimized_node_list[i][2];
    elems_Tet4.back()->set_node(3) = optimized_node_list[i][3];
    elems_Tet4.back()->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      if (rotated_tet_face_indices[i][j] < 6)
      {
        for (const auto & side_info : elem_side_list[rotated_tet_face_indices[i][j]])
          boundary_info.add_side(elems_Tet4.back(), j, side_info);
      }
    }
  }

  // Retain element extra integers
  for (unsigned int i = 0; i < 2; i++)
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elems_Tet4[i]->set_extra_integer(j, exist_extra_ids[j]);
    }
}

void
prismElemSplitter(ReplicatedMesh & mesh,
                  const dof_id_type elem_id,
                  std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(5);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[std::get<1>(bdry_side_list[i])].push_back(std::get<2>(bdry_side_list[i]));
    }
  }

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);

  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                        mesh.elem_ptr(elem_id)->node_ptr(1),
                                        mesh.elem_ptr(elem_id)->node_ptr(2),
                                        mesh.elem_ptr(elem_id)->node_ptr(3),
                                        mesh.elem_ptr(elem_id)->node_ptr(4),
                                        mesh.elem_ptr(elem_id)->node_ptr(5)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  auto optimized_node_list = prismNodeOptimizer(elem_node_list, rotated_tet_face_indices);

  std::vector<Elem *> elems_Tet4;
  for (unsigned int i = 0; i < optimized_node_list.size(); i++)
  {
    elems_Tet4.push_back(mesh.add_elem(new Tet4));
    elems_Tet4.back()->set_node(0) = optimized_node_list[i][0];
    elems_Tet4.back()->set_node(1) = optimized_node_list[i][1];
    elems_Tet4.back()->set_node(2) = optimized_node_list[i][2];
    elems_Tet4.back()->set_node(3) = optimized_node_list[i][3];
    elems_Tet4.back()->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      if (rotated_tet_face_indices[i][j] < 5)
      {
        for (const auto & side_info : elem_side_list[rotated_tet_face_indices[i][j]])
          boundary_info.add_side(elems_Tet4.back(), j, side_info);
      }
    }
  }

  // Retain element extra integers
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elems_Tet4[i]->set_extra_integer(j, exist_extra_ids[j]);
    }
}

void
pyramidElemSplitter(ReplicatedMesh & mesh,
                    const dof_id_type elem_id,
                    std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(5);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[std::get<1>(bdry_side_list[i])].push_back(std::get<2>(bdry_side_list[i]));
    }
  }

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                        mesh.elem_ptr(elem_id)->node_ptr(1),
                                        mesh.elem_ptr(elem_id)->node_ptr(2),
                                        mesh.elem_ptr(elem_id)->node_ptr(3),
                                        mesh.elem_ptr(elem_id)->node_ptr(4)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  auto optimized_node_list = pyramidNodeOptimizer(elem_node_list, rotated_tet_face_indices);

  std::vector<Elem *> elems_Tet4;
  for (unsigned int i = 0; i < optimized_node_list.size(); i++)
  {
    elems_Tet4.push_back(mesh.add_elem(new Tet4));
    elems_Tet4.back()->set_node(0) = optimized_node_list[i][0];
    elems_Tet4.back()->set_node(1) = optimized_node_list[i][1];
    elems_Tet4.back()->set_node(2) = optimized_node_list[i][2];
    elems_Tet4.back()->set_node(3) = optimized_node_list[i][3];
    elems_Tet4.back()->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      if (rotated_tet_face_indices[i][j] < 5)
      {
        for (const auto & side_info : elem_side_list[rotated_tet_face_indices[i][j]])
          boundary_info.add_side(elems_Tet4.back(), j, side_info);
      }
    }
  }

  // Retain element extra integers
  for (unsigned int i = 0; i < 2; i++)
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elems_Tet4[i]->set_extra_integer(j, exist_extra_ids[j]);
    }
}

std::vector<unsigned int>
neighborNodeIndicesHEX8(unsigned int min_id_index)
{
  const std::vector<std::vector<unsigned int>> preset_indices = {
      {1, 3, 4}, {0, 2, 5}, {3, 1, 6}, {2, 0, 7}, {5, 7, 0}, {4, 6, 1}, {7, 5, 2}, {6, 4, 3}};
  if (min_id_index > 7)
    mooseError("The input node index is out of range.");
  else
    return preset_indices[min_id_index];
}

std::vector<std::vector<Node *>>
hexNodeOptimizer(std::vector<Node *> & hex_nodes,
                 std::vector<std::vector<unsigned int>> & rotated_tet_face_indices)
{
  // Find the node with the minimum id
  std::vector<dof_id_type> node_ids(8);
  for (unsigned int i = 0; i < 8; i++)
    node_ids[i] = hex_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));
  const auto neighbor_node_indices = neighborNodeIndicesHEX8(min_node_id_index);

  const auto neighbor_node_ids = {node_ids[neighbor_node_indices[0]],
                                  node_ids[neighbor_node_indices[1]],
                                  node_ids[neighbor_node_indices[2]]};
  const unsigned int sec_min_pos =
      std::distance(std::begin(neighbor_node_ids),
                    std::min_element(std::begin(neighbor_node_ids), std::end(neighbor_node_ids)));

  std::vector<unsigned int> face_rotation;
  const auto rotated_indices = nodeRotationHEX8(min_node_id_index, sec_min_pos, face_rotation);
  std::vector<Node *> rotated_hex_nodes;
  for (unsigned int i = 0; i < 8; i++)
    rotated_hex_nodes.push_back(hex_nodes[rotated_indices[i]]);

  const auto diagonal_directions = quadFaceDiagonalDirectionsHex(rotated_hex_nodes);

  std::vector<std::vector<unsigned int>> tet_face_indices;
  const auto tet_nodes_set = tetNodesForHex(diagonal_directions, tet_face_indices);
  for (const auto & tet_face_index : tet_face_indices)
  {
    rotated_tet_face_indices.push_back(std::vector<unsigned int>());
    for (const auto & face_index : tet_face_index)
    {
      if (face_index < 6)
        rotated_tet_face_indices.back().push_back(face_rotation[face_index]);
      else
        rotated_tet_face_indices.back().push_back(6);
    }
  }

  std::vector<std::vector<Node *>> tet_nodes_list;
  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_hex_nodes[tet_node]);
  }

  return tet_nodes_list;
}

std::vector<bool>
quadFaceDiagonalDirectionsHex(std::vector<Node *> & hex_nodes)
{
  // Bottom/Top; Front/Back; Right/Left
  const std::vector<std::vector<unsigned int>> face_indices = {
      {0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 5, 4}, {2, 3, 7, 6}, {1, 2, 6, 5}, {3, 0, 4, 7}};
  std::vector<bool> diagonal_directions;
  for (const auto & face_index : face_indices)
  {
    std::vector<Node *> quad_nodes = {hex_nodes[face_index[0]],
                                      hex_nodes[face_index[1]],
                                      hex_nodes[face_index[2]],
                                      hex_nodes[face_index[3]]};
    diagonal_directions.push_back(quadFaceDiagonalDirection(quad_nodes));
  }
  return diagonal_directions;
}

bool
quadFaceDiagonalDirection(std::vector<Node *> & quad_nodes)
{
  const std::vector<dof_id_type> node_ids = {
      quad_nodes[0]->id(), quad_nodes[1]->id(), quad_nodes[2]->id(), quad_nodes[3]->id()};
  const unsigned int min_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));
  if (min_id_index == 0 || min_id_index == 2)
    return true;
  else
    return false;
}

std::vector<std::vector<unsigned int>>
tetNodesForHex(const std::vector<bool> diagonal_directions,
               std::vector<std::vector<unsigned int>> & tet_face_indices)
{
  const std::vector<std::vector<bool>> possible_inputs = {{true, true, true, true, true, false},
                                                          {true, true, true, true, false, false},
                                                          {true, true, true, false, true, false},
                                                          {true, false, true, true, true, false},
                                                          {true, false, true, true, false, false},
                                                          {true, false, true, false, true, false},
                                                          {true, false, true, false, false, false}};

  const unsigned int input_index = std::distance(
      std::begin(possible_inputs),
      std::find(std::begin(possible_inputs), std::end(possible_inputs), diagonal_directions));

  switch (input_index)
  {
    case 0:
      tet_face_indices = {
          {0, 6, 2, 6}, {1, 6, 2, 6}, {1, 6, 5, 6}, {0, 6, 3, 4}, {6, 6, 3, 6}, {6, 4, 5, 6}};
      return {{0, 1, 2, 6}, {0, 5, 1, 6}, {0, 4, 5, 6}, {0, 2, 3, 7}, {0, 6, 2, 7}, {0, 4, 6, 7}};
    case 1:
      tet_face_indices = {
          {0, 1, 2, 6}, {6, 6, 2, 6}, {6, 6, 5, 1}, {0, 6, 3, 4}, {6, 6, 3, 6}, {6, 4, 5, 6}};
      return {{0, 1, 2, 5}, {0, 2, 6, 5}, {0, 6, 4, 5}, {0, 2, 3, 7}, {0, 6, 2, 7}, {0, 4, 6, 7}};
    case 2:
      tet_face_indices = {
          {0, 6, 2, 6}, {1, 6, 2, 6}, {1, 6, 5, 6}, {4, 6, 5, 6}, {4, 6, 3, 6}, {0, 6, 3, 6}};
      return {{0, 1, 2, 6}, {0, 5, 1, 6}, {0, 4, 5, 6}, {0, 7, 4, 6}, {0, 3, 7, 6}, {0, 2, 3, 6}};
    case 3:
      tet_face_indices = {
          {4, 6, 5, 1}, {6, 6, 5, 6}, {6, 1, 2, 6}, {4, 0, 3, 6}, {6, 6, 3, 6}, {6, 6, 2, 0}};
      return {{0, 7, 4, 5}, {0, 6, 7, 5}, {0, 1, 6, 5}, {0, 3, 7, 2}, {0, 7, 6, 2}, {0, 6, 1, 2}};
    case 4:
      tet_face_indices = {{0, 1, 2, 6}, {0, 6, 3, 4}, {5, 4, 6, 1}, {5, 6, 3, 2}, {6, 6, 6, 6}};
      return {{0, 1, 2, 5}, {0, 2, 3, 7}, {4, 7, 5, 0}, {5, 7, 6, 2}, {0, 2, 7, 5}};
    case 5:
      tet_face_indices = {
          {4, 6, 5, 1}, {6, 6, 5, 6}, {6, 1, 2, 6}, {2, 6, 6, 0}, {3, 6, 6, 0}, {3, 6, 6, 4}};
      return {{0, 7, 4, 5}, {0, 6, 7, 5}, {0, 1, 6, 5}, {1, 6, 2, 0}, {2, 6, 3, 0}, {3, 6, 7, 0}};
    case 6:
      tet_face_indices = {
          {1, 4, 5, 6}, {6, 6, 5, 6}, {6, 6, 3, 4}, {1, 6, 2, 0}, {6, 6, 2, 6}, {6, 0, 3, 6}};
      return {{0, 4, 5, 7}, {0, 5, 6, 7}, {0, 6, 3, 7}, {0, 5, 1, 2}, {0, 6, 5, 2}, {0, 3, 6, 2}};
    default:
      mooseError("Unexpected input.");
  }
}

std::vector<unsigned int>
nodeRotationHEX8(unsigned int min_id_index,
                 unsigned int sec_min_pos,
                 std::vector<unsigned int> & face_rotation)
{
  const std::vector<std::vector<std::vector<unsigned int>>> preset_indices = {
      {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 3, 7, 4, 1, 2, 6, 5}, {0, 4, 5, 1, 3, 7, 6, 2}},
      {{1, 0, 4, 5, 2, 3, 7, 6}, {1, 2, 3, 0, 5, 6, 7, 4}, {1, 5, 6, 2, 0, 4, 7, 3}},
      {{2, 3, 0, 1, 6, 7, 4, 5}, {2, 1, 5, 6, 3, 0, 4, 7}, {2, 6, 7, 3, 1, 5, 4, 0}},
      {{3, 2, 6, 7, 0, 1, 5, 4}, {3, 0, 1, 2, 7, 4, 5, 6}, {3, 7, 4, 0, 2, 6, 5, 1}},
      {{4, 5, 1, 0, 7, 6, 2, 3}, {4, 7, 6, 5, 0, 3, 2, 1}, {4, 0, 3, 7, 5, 1, 2, 6}},
      {{5, 4, 7, 6, 1, 0, 3, 2}, {5, 6, 2, 1, 4, 7, 3, 0}, {5, 1, 0, 4, 6, 2, 3, 7}},
      {{6, 7, 3, 2, 5, 4, 0, 1}, {6, 5, 4, 7, 2, 1, 0, 3}, {6, 2, 1, 5, 7, 3, 0, 4}},
      {{7, 6, 5, 4, 3, 2, 1, 0}, {7, 4, 0, 3, 6, 5, 1, 2}, {7, 3, 2, 6, 4, 0, 1, 5}}};

  const std::vector<std::vector<std::vector<unsigned int>>> preset_face_indices = {
      {{0, 1, 2, 3, 4, 5}, {4, 0, 3, 5, 1, 2}, {1, 4, 5, 2, 0, 3}},
      {{1, 0, 4, 5, 2, 3}, {0, 2, 3, 4, 1, 5}, {2, 1, 5, 3, 0, 4}},
      {{0, 3, 4, 1, 2, 5}, {2, 0, 1, 5, 3, 4}, {3, 2, 5, 4, 0, 1}},
      {{3, 0, 2, 5, 4, 5}, {0, 4, 1, 2, 3, 5}, {4, 3, 5, 1, 0, 2}},
      {{2, 5, 2, 0, 4, 3}, {5, 4, 3, 2, 2, 0}, {4, 1, 0, 3, 5, 2}},
      {{5, 1, 4, 3, 5, 0}, {2, 5, 3, 0, 1, 4}, {1, 2, 0, 4, 5, 3}},
      {{3, 5, 4, 0, 2, 1}, {5, 2, 1, 4, 3, 0}, {2, 3, 0, 1, 5, 4}},
      {{5, 3, 2, 1, 4, 0}, {4, 5, 1, 0, 3, 2}, {3, 4, 0, 2, 5, 1}}};

  if (min_id_index > 7 || sec_min_pos > 2)
    mooseError("The input node index is out of range.");
  else
  {
    // index: new face index; value: old face index
    face_rotation = preset_face_indices[min_id_index][sec_min_pos];
    return preset_indices[min_id_index][sec_min_pos];
  }
}

std::vector<unsigned int>
nodeRotationPRISM6(unsigned int min_id_index, std::vector<unsigned int> & face_rotation)
{
  const std::vector<std::vector<unsigned int>> preset_indices = {{0, 1, 2, 3, 4, 5},
                                                                 {1, 2, 0, 4, 5, 3},
                                                                 {2, 0, 1, 5, 3, 4},
                                                                 {3, 5, 4, 0, 2, 1},
                                                                 {4, 3, 5, 1, 0, 2},
                                                                 {5, 4, 3, 2, 1, 0}};

  const std::vector<std::vector<unsigned int>> preset_face_indices = {{0, 1, 2, 3, 4},
                                                                      {0, 2, 3, 1, 4},
                                                                      {0, 3, 1, 2, 4},
                                                                      {4, 3, 2, 1, 0},
                                                                      {4, 1, 3, 2, 0},
                                                                      {4, 2, 1, 3, 0}};

  if (min_id_index > 5)
    mooseError("The input node index is out of range.");
  else
  {
    // index: new face index; value: old face index
    face_rotation = preset_face_indices[min_id_index];
    return preset_indices[min_id_index];
  }
}

std::vector<std::vector<Node *>>
prismNodeOptimizer(std::vector<Node *> & prism_nodes,
                   std::vector<std::vector<unsigned int>> & rotated_tet_face_indices)
{
  // Find the node with the minimum id
  std::vector<dof_id_type> node_ids(6);
  for (unsigned int i = 0; i < 6; i++)
    node_ids[i] = prism_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));

  std::vector<unsigned int> face_rotation;
  const auto rotated_indices = nodeRotationPRISM6(min_node_id_index, face_rotation);
  std::vector<Node *> rotated_prism_nodes;
  for (unsigned int i = 0; i < 6; i++)
    rotated_prism_nodes.push_back(prism_nodes[rotated_indices[i]]);

  std::vector<Node *> key_quad_nodes = {rotated_prism_nodes[1],
                                        rotated_prism_nodes[2],
                                        rotated_prism_nodes[5],
                                        rotated_prism_nodes[4]};

  const bool diagonal_direction = quadFaceDiagonalDirection(key_quad_nodes);

  std::vector<std::vector<unsigned int>> tet_face_indices;
  const auto tet_nodes_set = tetNodesForPrism(diagonal_direction, tet_face_indices);
  for (const auto & tet_face_index : tet_face_indices)
  {
    rotated_tet_face_indices.push_back(std::vector<unsigned int>());
    for (const auto & face_index : tet_face_index)
    {
      if (face_index < 5)
        rotated_tet_face_indices.back().push_back(face_rotation[face_index]);
      else
        rotated_tet_face_indices.back().push_back(5);
    }
  }

  std::vector<std::vector<Node *>> tet_nodes_list;
  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_prism_nodes[tet_node]);
  }

  return tet_nodes_list;
}

std::vector<std::vector<unsigned int>>
tetNodesForPrism(const bool diagonal_direction,
                 std::vector<std::vector<unsigned int>> & tet_face_indices)
{

  if (diagonal_direction)
  {
    tet_face_indices = {{4, 3, 5, 1}, {2, 1, 5, 5}, {2, 5, 3, 0}};
    return {{3, 5, 4, 0}, {1, 4, 5, 0}, {1, 5, 2, 0}};
  }
  else
  {
    tet_face_indices = {{4, 3, 5, 1}, {2, 1, 5, 0}, {2, 5, 5, 3}};
    return {{3, 5, 4, 0}, {1, 4, 2, 0}, {2, 4, 5, 0}};
  }
}

std::vector<unsigned int>
nodeRotationPYRAMIND5(unsigned int min_id_index, std::vector<unsigned int> & face_rotation)
{
  const std::vector<std::vector<unsigned int>> preset_indices = {
      {0, 1, 2, 3, 4}, {1, 2, 3, 0, 4}, {2, 3, 0, 1, 4}, {3, 0, 1, 2, 4}};

  const std::vector<std::vector<unsigned int>> preset_face_indices = {
      {0, 1, 2, 3, 4}, {1, 2, 3, 0, 4}, {2, 3, 0, 1, 4}, {3, 0, 1, 2, 4}};

  if (min_id_index > 3)
    mooseError("The input node index is out of range.");
  else
  {
    // index: new face index; value: old face index
    face_rotation = preset_face_indices[min_id_index];
    return preset_indices[min_id_index];
  }
}

std::vector<std::vector<Node *>>
pyramidNodeOptimizer(std::vector<Node *> & pyramid_nodes,
                     std::vector<std::vector<unsigned int>> & rotated_tet_face_indices)
{
  // Find the node with the minimum id, ignoring the top node
  std::vector<dof_id_type> node_ids(4);
  for (unsigned int i = 0; i < 4; i++)
    node_ids[i] = pyramid_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));

  std::vector<unsigned int> face_rotation;
  const auto rotated_indices = nodeRotationPYRAMIND5(min_node_id_index, face_rotation);
  std::vector<Node *> rotated_pyramid_nodes;
  for (unsigned int i = 0; i < 5; i++)
    rotated_pyramid_nodes.push_back(pyramid_nodes[rotated_indices[i]]);

  const std::vector<std::vector<unsigned int>> tet_nodes_set = {{0, 1, 2, 4}, {0, 2, 3, 4}};
  const std::vector<std::vector<unsigned int>> tet_face_indices = {{4, 0, 1, 5}, {4, 5, 2, 3}};

  for (const auto & tet_face_index : tet_face_indices)
  {
    rotated_tet_face_indices.push_back(std::vector<unsigned int>());
    for (const auto & face_index : tet_face_index)
    {
      if (face_index < 5)
        rotated_tet_face_indices.back().push_back(face_rotation[face_index]);
      else
        rotated_tet_face_indices.back().push_back(5);
    }
  }

  std::vector<std::vector<Node *>> tet_nodes_list;
  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_pyramid_nodes[tet_node]);
  }

  return tet_nodes_list;
}

void
convert3DMeshToAllTet4(ReplicatedMesh & mesh,
                       const std::vector<std::pair<dof_id_type, bool>> & elems_to_process,
                       std::vector<dof_id_type> & converted_elems_ids_to_cut,
                       std::vector<dof_id_type> & converted_elems_ids_to_retain,
                       const subdomain_id_type & block_id_to_remove,
                       const bool delete_block_to_remove)
{
  for (const auto & elem_to_process : elems_to_process)
  {
    switch (mesh.elem_ptr(elem_to_process.first)->type())
    {
      case ElemType::HEX8:
        hexElemSplitter(mesh,
                        elem_to_process.first,
                        elem_to_process.second ? converted_elems_ids_to_cut
                                               : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::PYRAMID5:
        pyramidElemSplitter(mesh,
                            elem_to_process.first,
                            elem_to_process.second ? converted_elems_ids_to_cut
                                                   : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::PRISM6:
        prismElemSplitter(mesh,
                          elem_to_process.first,
                          elem_to_process.second ? converted_elems_ids_to_cut
                                                 : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::TET4:
        if (elem_to_process.second)
          converted_elems_ids_to_cut.push_back(elem_to_process.first);
        else
          converted_elems_ids_to_retain.push_back(elem_to_process.first);
        break;
      default:
        mooseError("Unexpected element type.");
    }
  }

  if (delete_block_to_remove)
  {
    for (auto elem_it = mesh.active_subdomain_elements_begin(block_id_to_remove);
         elem_it != mesh.active_subdomain_elements_end(block_id_to_remove);
         elem_it++)
      mesh.delete_elem(*elem_it);

    mesh.contract();
    mesh.prepare_for_use();
  }
}

void
convert3DMeshToAllTet4(ReplicatedMesh & mesh)
{
  // Subdomain ID for new utility blocks must be new
  std::set<subdomain_id_type> subdomain_ids_set;
  mesh.subdomain_ids(subdomain_ids_set);
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;
  std::vector<std::pair<dof_id_type, bool>> original_elems;

  for (auto elem_it = mesh.active_elements_begin(); elem_it != mesh.active_elements_end();
       elem_it++)
  {
    if ((*elem_it)->default_order() != Order::FIRST)
      mooseError("Only first order elements are supported for cutting.");
    original_elems.push_back(std::make_pair((*elem_it)->id(), false));
  }

  std::vector<dof_id_type> converted_elems_ids_to_cut;
  std::vector<dof_id_type> converted_elems_ids_to_retain;

  convert3DMeshToAllTet4(mesh,
                         original_elems,
                         converted_elems_ids_to_cut,
                         converted_elems_ids_to_retain,
                         block_id_to_remove,
                         true);
}
}
