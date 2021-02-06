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

    subdomain_id_type id = Moose::INVALID_BLOCK_ID;
    std::istringstream ss(subdomain_name[i]);

    if (!(ss >> id) || !ss.eof())
      id = mesh.get_id_by_name(subdomain_name[i]);

    ids[i] = id;
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
}
