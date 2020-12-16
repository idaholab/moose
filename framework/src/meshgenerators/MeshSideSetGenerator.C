//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshSideSetGenerator.h"
#include "BndElement.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <typeinfo>

registerMooseObject("MooseApp", MeshSideSetGenerator);

defineLegacyParams(MeshSideSetGenerator);

InputParameters
MeshSideSetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Add lower dimensional elements along the faces contained in a side "
                             "set to set up mixed dimensional problems");
  params.addRequiredParam<std::vector<BoundaryName>>("boundaries",
                                                     "The name of the boundary to mesh");
  params.addRequiredParam<subdomain_id_type>(
      "block_id", "Subdomain id to set for the new elements along the boundary");
  params.addParam<SubdomainName>(
      "block_name", "Subdomain name to set for the new elements along the boundary (optional)");

  return params;
}

MeshSideSetGenerator::MeshSideSetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _block_id(parameters.get<subdomain_id_type>("block_id"))
{
  if (typeid(_input).name() == typeid(std::unique_ptr<DistributedMesh>).name())
    mooseError("MeshSideSetGenerator only works with ReplicatedMesh");
}

std::unique_ptr<MeshBase>
MeshSideSetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  auto & boundary_info = mesh->get_boundary_info();

  // get IDs of all boundaries with which a new block is created
  std::set<boundary_id_type> mesh_boundary_ids;
  if (isParamValid("boundaries"))
  {
    auto & boundary_names = getParam<std::vector<BoundaryName>>("boundaries");
    for (auto & boundary_name : boundary_names)
      mesh_boundary_ids.insert(boundary_info.get_id_by_name(boundary_name));
  }
  else
    mesh_boundary_ids = boundary_info.get_boundary_ids();

  // Equivalent to MooseMesh::buildBndElemList()
  auto bc_tuples = boundary_info.build_active_side_list();
  int n = bc_tuples.size();
  std::vector<std::unique_ptr<BndElement>> bnd_elems;
  std::map<boundary_id_type, std::set<dof_id_type>> bnd_elem_ids;
  bnd_elems.reserve(n);
  for (const auto & t : bc_tuples)
  {
    auto elem_id = std::get<0>(t);
    auto side_id = std::get<1>(t);
    auto bc_id = std::get<2>(t);

    std::unique_ptr<BndElement> bndElem =
        libmesh_make_unique<BndElement>(mesh->elem_ptr(elem_id), side_id, bc_id);
    bnd_elems.push_back(std::move(bndElem));
    bnd_elem_ids[bc_id].insert(elem_id);
  }

  ////// PART TO BE ABLE TO ASSIGN BC TO LOWERDIM ON EXTERNAL BOUNDARIES
  /*
  // get IDs of all other boundaries
  std::set<boundary_id_type> other_boundary_ids;
  for (auto & bnd_id : boundary_info.get_boundary_ids())
    if(mesh_boundary_ids.find(bnd_id) == mesh_boundary_ids.end())
      other_boundary_ids.insert(bnd_id);
  // Identify node coords and their IDs on other boundaries
  std::set<std::pair<const Node *, unsigned int>> boundary_node_coords;
  for (auto it = bnd_elems.begin(); it != bnd_elems.end(); ++it)
    if (other_boundary_ids.count((*it)->_bnd_id) > 0)
    {
      Elem * elem = (*it)->_elem;
      auto s = (*it)->_side;
      std::vector<unsigned int> nodes_on_side = elem->nodes_on_side(s);
      for (unsigned int n = 0; n < nodes_on_side.size(); ++n)
        boundary_node_coords.insert(std::make_pair(elem->node_ptr(nodes_on_side[n]),(*it)->_bnd_id));
    }
  //*/

  for (auto it = bnd_elems.begin(); it != bnd_elems.end(); ++it)
    if (mesh_boundary_ids.count((*it)->_bnd_id) > 0)
    {
      Elem * elem = (*it)->_elem;
      auto s = (*it)->_side;
      Elem * neighbor = elem->neighbor_ptr(s);

      if (neighbor &&
          elem->id() >
              neighbor->id()) // test so that lowerdim elem are not duplicated on internal
                              // boundaries //UPDATE this will fail for SideSetsBetweenSubdomains
      {
        // build element from the side
        std::unique_ptr<Elem> side(elem->build_side_ptr(s, false));
        side->processor_id() = elem->processor_id();

        // Add the side set subdomain
        Elem * new_elem = mesh->add_elem(side.release());
        new_elem->subdomain_id() = _block_id;

        // update elem-neighbor connections:
        // the new neighbor of "elem" is the lowerdim element
        elem->set_neighbor(s, new_elem);
        // the new neighbor of "neighbor" is the lowerdim element
        for (unsigned int s_i = 0; s_i < neighbor->n_sides(); ++s_i)
        {
          side = neighbor->build_side_ptr(s_i, false);
          if (side->centroid() == new_elem->centroid())
            neighbor->set_neighbor(s_i, new_elem);
        }

        ////// PART TO BE ABLE TO ASSIGN BC TO LOWERDIM ON EXTERNAL BOUNDARIES
        /*
        // add node on external boundaries to a new "LD_XXX" boundary nodeset with an id 100+
        for (unsigned int s_i = 0; s_i < new_elem->n_sides(); ++s_i)
        {
          auto side_centroid = new_elem->build_side_ptr(s_i, false)->centroid();
          for (auto it = boundary_node_coords.begin(); it != boundary_node_coords.end(); ++it)
            if (std::get<0>(*it)->absolute_fuzzy_equals(side_centroid,1e-5))
              {
                boundary_info.add_side(new_elem->id(), s_i, 100+std::get<1>(*it));
                boundary_info.nodeset_name(100+std::get<1>(*it)) = "LD_" +
        boundary_info.sideset_name(std::get<1>(*it));
              }
        }
        //*/
      }
    }

  // Loop to assign the neighbors of all the new lowerdim elements
  // for each element in the lower dim block, we check if the elem has neighbor by finding other
  // elements that share a side at the same position
  for (auto & elem_i : mesh->element_ptr_range())
    if (elem_i->subdomain_id() == _block_id)
      for (unsigned int s_i = 0; s_i < elem_i->n_sides(); ++s_i)
      {
        std::unique_ptr<Elem> face_i(elem_i->build_side_ptr(s_i, false));
        std::vector<std::pair<unsigned int, Elem *>> neighbors;
        neighbors.push_back(std::make_pair(s_i, elem_i));
        for (auto & elem_j : mesh->element_ptr_range())
          if (elem_j->subdomain_id() == _block_id && elem_j != elem_i)
            for (unsigned int s_j = 0; s_j < elem_j->n_sides(); ++s_j)
            {
              std::unique_ptr<Elem> face_j(elem_j->build_side_ptr(s_j, false));
              if (face_i->centroid() == face_j->centroid())
                neighbors.push_back(std::make_pair(s_j, elem_j));
            }
        if (neighbors.size() > 1) // then we have found neighbors to the element
        {
          // the implementation below (along with the modification in MooseMesh) allows to take into
          // account all the intersections a lower dim can have in that case, all the neighbors to
          // one lowerdim element at an intersection are taken into account by doing a loop of
          // unique elem-neighbor pairs
          for (unsigned int i = 0; i < neighbors.size() - 1; ++i)
            std::get<1>(neighbors[i])
                ->set_neighbor(std::get<0>(neighbors[i]), std::get<1>(neighbors[i + 1]));
          // loop back on the the element considered to have of unique connections at a lowerdim
          // intersection
          std::get<1>(neighbors.back())
              ->set_neighbor(std::get<0>(neighbors.back()), std::get<1>(neighbors[0]));
          // note that with this current implementation the same assignment is overwritten multiple
          // times
        }
      }

  // Assign block name, if provided
  if (isParamValid("block_name"))
    mesh->subdomain_name(_block_id) = getParam<SubdomainName>("block_name");

  return dynamic_pointer_cast<MeshBase>(mesh);
}
