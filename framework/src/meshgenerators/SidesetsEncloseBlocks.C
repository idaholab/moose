//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SidesetsEncloseBlocks.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/remote_elem.h"

registerMooseObject("MooseApp", SidesetsEncloseBlocks);

InputParameters
SidesetsEncloseBlocks::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<SubdomainName>>(
      "block", "The set of blocks that are checked if they are enclosed by boundary");
  params.addParam<std::vector<BoundaryName>>("boundary",
                                             "The name of the boundaries enclosing the ");
  params.addParam<BoundaryName>("new_boundary", "The name of the boundary to create");
  params.addClassDescription(
      "MeshGenerator that checks if a set of blocks is enclosed by a set of sidesets."
      "It can either add sides that are not covered by a sideset by a new sidesets or"
      "error out.");

  return params;
}

SidesetsEncloseBlocks::SidesetsEncloseBlocks(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
SidesetsEncloseBlocks::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // error on finding a side that is not covered
  bool error_out = !isParamValid("new_boundary");
  boundary_id_type new_sideset_id;
  if (!error_out)
  {
    BoundaryName new_boundary = getParam<BoundaryName>("new_boundary");
    std::stringstream ss;
    ss << new_boundary;
    ss >> new_sideset_id;
    if (ss.fail())
      new_sideset_id = boundary_info.get_id_by_name(new_boundary);

    if (new_sideset_id == BoundaryInfo::invalid_id)
      paramError("new_boundary", "Not a valid boundary");
  }

  std::vector<subdomain_id_type> vec_block_ids =
      MooseMeshUtils::getSubdomainIDs(*mesh, getParam<std::vector<SubdomainName>>("block"));
  std::set<subdomain_id_type> blk_ids(vec_block_ids.begin(), vec_block_ids.end());

  // get boundaries
  // check if the provided sideset name is actually a sideset id
  // if it can be converted to integer it's interpreted
  // as a sideset id
  auto boundary_name_vec = getParam<std::vector<BoundaryName>>("boundary");
  std::vector<boundary_id_type> bnd_ids_vec;
  bnd_ids_vec.reserve(boundary_name_vec.size());
  std::set<boundary_id_type> bnd_ids;
  for (auto & bnd_name : boundary_name_vec)
  {
    std::stringstream ss;
    boundary_id_type sideset_id;
    ss << bnd_name;
    ss >> sideset_id;
    if (ss.fail())
      sideset_id = boundary_info.get_id_by_name(bnd_name);

    if (sideset_id == BoundaryInfo::invalid_id)
      paramError("boundary", "Not a valid boundary");
    bnd_ids_vec.push_back(sideset_id);
    bnd_ids.insert(sideset_id);
  }

  // loop over all elements in blocks and for each elem over each side
  // and check the neighbors
  for (auto & block_id : blk_ids)
  {
    for (const Elem * elem : as_range(mesh->active_local_subdomain_elements_begin(block_id),
                                      mesh->active_local_subdomain_elements_end(block_id)))
    {
      for (unsigned int j = 0; j < elem->n_sides(); ++j)
      {
        const Elem * neigh = elem->neighbor_ptr(j);

        // is this an outside boundary to blocks?
        // NOTE: the next line is NOT sufficient for AMR!!
        bool is_outer_bnd = !neigh || blk_ids.find(neigh->subdomain_id()) == blk_ids.end();
        if (is_outer_bnd)
        {
          // get all boundary ids of this side, then compare the set of these boundary_ids
          // to the set of boundary_ids provided to the mesh generator; if the intersection
          // is empty then this side is NOT convered
          std::vector<boundary_id_type> side_boundary_ids_vec;
          boundary_info.raw_boundary_ids(elem, j, side_boundary_ids_vec);

          std::set<boundary_id_type> intersection;
          std::set_intersection(side_boundary_ids_vec.begin(),
                                side_boundary_ids_vec.end(),
                                bnd_ids_vec.begin(),
                                bnd_ids_vec.end(),
                                std::inserter(intersection, intersection.end()));

          if (intersection.size() == 0)
          {
            if (error_out)
              mooseError("Element id ",
                         elem->id(),
                         " side ",
                         j,
                         " is external and not covered by specified boundaries.");
            else
              boundary_info.add_side(elem, j, new_sideset_id);
          }
        }
      }
    }
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
