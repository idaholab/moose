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
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_base.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"

namespace MooseMeshUtils
{
void
changeBoundaryId(MeshBase & mesh,
                 const libMesh::boundary_id_type old_id,
                 const libMesh::boundary_id_type new_id,
                 bool delete_prev)
{
  // Get a reference to our BoundaryInfo object, we will use it several times below...
  libMesh::BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Container to catch ids passed back from BoundaryInfo
  std::vector<libMesh::boundary_id_type> old_ids;

  // Only level-0 elements store BCs.  Loop over them.
  for (auto & elem : as_range(mesh.level_elements_begin(0), mesh.level_elements_end(0)))
  {
    unsigned int n_sides = elem->n_sides();
    for (unsigned int s = 0; s != n_sides; ++s)
    {
      boundary_info.boundary_ids(elem, s, old_ids);
      if (std::find(old_ids.begin(), old_ids.end(), old_id) != old_ids.end())
      {
        std::vector<libMesh::boundary_id_type> new_ids(old_ids);
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
}

std::vector<libMesh::boundary_id_type>
getBoundaryIDs(const libMesh::MeshBase & mesh,
               const std::vector<BoundaryName> & boundary_name,
               bool generate_unknown)
{
  const libMesh::BoundaryInfo & boundary_info = mesh.get_boundary_info();
  const std::map<libMesh::boundary_id_type, std::string> & sideset_map =
      boundary_info.get_sideset_name_map();
  const std::map<libMesh::boundary_id_type, std::string> & nodeset_map =
      boundary_info.get_nodeset_name_map();

  std::set<libMesh::boundary_id_type> boundary_ids = boundary_info.get_boundary_ids();

  // On a distributed mesh we may have boundary ids that only exist on
  // other processors.
  if (!mesh.is_replicated())
    mesh.comm().set_union(boundary_ids);

  libMesh::boundary_id_type max_boundary_id = boundary_ids.empty() ? 0 : *(boundary_ids.rbegin());

  std::vector<libMesh::boundary_id_type> ids(boundary_name.size());
  for (unsigned int i = 0; i < boundary_name.size(); i++)
  {
    if (boundary_name[i] == "ANY_BOUNDARY_ID")
    {
      ids.assign(boundary_ids.begin(), boundary_ids.end());
      if (i)
        mooseWarning("You passed \"ANY_BOUNDARY_ID\" in addition to other boundary_names.  This "
                     "may be a logic error.");
      break;
    }

    libMesh::boundary_id_type id;
    std::istringstream ss(boundary_name[i]);

    if (!(ss >> id) || !ss.eof())
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

    ids[i] = id;
  }

  return ids;
}

std::vector<subdomain_id_type>
getSubdomainIDs(const libMesh::MeshBase & mesh, const std::vector<SubdomainName> & subdomain_name)
{
  std::vector<subdomain_id_type> ids(subdomain_name.size());
  std::set<subdomain_id_type> mesh_subdomains;
  mesh.subdomain_ids(mesh_subdomains);

  for (unsigned int i = 0; i < subdomain_name.size(); i++)
  {
    if (subdomain_name[i] == "ANY_BLOCK_ID")
    {
      ids.assign(mesh_subdomains.begin(), mesh_subdomains.end());
      if (i)
        mooseWarning("You passed \"ANY_BLOCK_ID\" in addition to other block names.  This may be a "
                     "logic error.");
      break;
    }

    ids[i] = MooseMeshUtils::getSubdomainID(subdomain_name[i], mesh);
  }

  return ids;
}

BoundaryID
getBoundaryID(const BoundaryName & boundary_name, const MeshBase & mesh)
{
  BoundaryID id = Moose::INVALID_BOUNDARY_ID;
  std::istringstream ss(boundary_name);

  if (!(ss >> id))
    id = mesh.get_boundary_info().get_id_by_name(boundary_name);

  return id;
}

SubdomainID
getSubdomainID(const SubdomainName & subdomain_name, const MeshBase & mesh)
{
  if (subdomain_name == "ANY_BLOCK_ID")
    mooseError("getSubdomainID() does not work with \"ANY_BLOCK_ID\"");

  SubdomainID id = Moose::INVALID_BLOCK_ID;
  std::istringstream ss(subdomain_name);

  if (!(ss >> id) || !ss.eof())
    id = mesh.get_id_by_name(subdomain_name);

  return id;
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

std::map<dof_id_type, dof_id_type>
getExtraIDUniqueCombinationMap(const MeshBase & mesh,
                               const std::set<SubdomainID> & block_ids,
                               std::vector<ExtraElementIDName> extra_ids)
{
  // check block restriction
  const bool block_restricted = block_ids.find(Moose::ANY_BLOCK_ID) == block_ids.end();
  // get element id name of interest in recursive parsing algorithm
  ExtraElementIDName id_name = extra_ids.back();
  extra_ids.pop_back();
  const auto id_index = mesh.get_elem_integer_index(id_name);
  // create base parsed id set
  if (extra_ids.empty())
  {
    // get set of extra id values;
    std::set<dof_id_type> ids;
    for (const auto & elem : mesh.active_local_element_ptr_range())
    {
      if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
        continue;
      auto id = elem->get_extra_integer(id_index);
      ids.insert(id);
    }
    mesh.comm().set_union(ids);
    // determine new extra id values;
    std::map<dof_id_type, dof_id_type> parsed_ids;
    for (auto & elem : mesh.active_local_element_ptr_range())
    {
      if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
        continue;
      parsed_ids[elem->id()] = std::distance(
          ids.begin(), std::find(ids.begin(), ids.end(), elem->get_extra_integer(id_index)));
    }
    return parsed_ids;
  }
  // if extra_ids is not empty, recursively call getExtraIDUniqueCombinationMap
  std::map<dof_id_type, dof_id_type> base_parsed_ids =
      MooseMeshUtils::getExtraIDUniqueCombinationMap(mesh, block_ids, extra_ids);
  // parsing extra ids based on ref_parsed_ids
  std::set<std::pair<dof_id_type, dof_id_type>> unique_ids;
  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
      continue;
    const dof_id_type id1 = base_parsed_ids[elem->id()];
    const dof_id_type id2 = elem->get_extra_integer(id_index);
    if (!unique_ids.count(std::pair<dof_id_type, dof_id_type>(id1, id2)))
      unique_ids.insert(std::pair<dof_id_type, dof_id_type>(id1, id2));
  }
  mesh.comm().set_union(unique_ids);
  std::map<dof_id_type, dof_id_type> parsed_ids;
  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    if (block_restricted && block_ids.find(elem->subdomain_id()) == block_ids.end())
      continue;
    const dof_id_type id1 = base_parsed_ids[elem->id()];
    const dof_id_type id2 = elem->get_extra_integer(id_index);
    parsed_ids[elem->id()] = std::distance(
        unique_ids.begin(),
        std::find(
            unique_ids.begin(), unique_ids.end(), std::pair<dof_id_type, dof_id_type>(id1, id2)));
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
}
