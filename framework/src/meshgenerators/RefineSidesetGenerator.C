//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RefineSidesetGenerator.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_refinement.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", RefineSidesetGenerator);

InputParameters
RefineSidesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Mesh generator which refines one or more sidesets");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to modify");
  params.addRequiredParam<std::vector<BoundaryName>>("boundaries",
                                                     "The list of boundaries to be modified");
  params.addRequiredParam<std::vector<int>>(
      "refinement",
      "The amount of times to refine each sideset, corresponding to their index in 'boundaries'");
  params.addParam<bool>(
      "enable_neighbor_refinement",
      true,
      "Toggles whether neighboring level one elements should be refined or not. Defaults to true");
  MultiMooseEnum boundary_side("primary secondary both", "both");
  params.addParam<MultiMooseEnum>("boundary_side",
                                  boundary_side,
                                  "Whether the generator should refine itself(primary), its "
                                  "neighbors(secondary), or itself and its neighbors(both)");

  return params;
}

RefineSidesetGenerator::RefineSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundaries(getParam<std::vector<BoundaryName>>("boundaries")),
    _refinement(getParam<std::vector<int>>("refinement")),
    _enable_neighbor_refinement(getParam<bool>("enable_neighbor_refinement")),
    _boundary_side(getParam<MultiMooseEnum>("boundary_side"))
{
}

std::unique_ptr<MeshBase>
RefineSidesetGenerator::generate()
{
  // Get the list of boundary ids from the boundary names
  const auto boundary_ids = MooseMeshUtils::getBoundaryIDs(
      *_input, getParam<std::vector<BoundaryName>>("boundaries"), false);

  // Check that the boundary ids/names exist in the mesh
  for (std::size_t i = 0; i < boundary_ids.size(); ++i)
    if (boundary_ids[i] == Moose::INVALID_BOUNDARY_ID)
      paramError("boundaries",
                 "The boundary '",
                 getParam<std::vector<BoundaryName>>("boundaries")[i],
                 "' was not found within the mesh");
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  int max = *std::max_element(_refinement.begin(), _refinement.end());
  return recursive_refine(boundary_ids, mesh, _refinement, max);
}

std::unique_ptr<MeshBase>
RefineSidesetGenerator::recursive_refine(std::vector<boundary_id_type> boundary_ids,
                                         std::unique_ptr<MeshBase> & mesh,
                                         std::vector<int> refinement,
                                         int max,
                                         int ref_step)
{
  // If the refinement step has reached the largest value in the _refinement array, return the mesh,
  // as we are done.
  if (ref_step == max)
    return dynamic_pointer_cast<MeshBase>(mesh);
  mesh->prepare_for_use();
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> sideList =
      mesh->get_boundary_info().build_active_side_list();
  for (std::size_t i = 0; i < boundary_ids.size(); i++)
  {
    if (refinement[i] > 0 && refinement[i] > ref_step)
    {
      for (std::tuple<dof_id_type, unsigned short int, boundary_id_type> tuple : sideList)
      {
        if (std::get<2>(tuple) == boundary_ids[i])
        {
          Elem * elem = mesh->elem_ptr(std::get<0>(tuple));
          if (_boundary_side[i] == "primary" || _boundary_side[i] == "both")
            elem->set_refinement_flag(Elem::REFINE);
          if (_boundary_side[i] == "secondary" || _boundary_side[i] == "both")
          {
            auto neighbor = elem->neighbor_ptr(std::get<1>(tuple));
            // when we have multiple domains, domains will only refine the elements that they own
            // since there can be instances where there is no neighbor, this null_ptr check is
            // necessary
            if (neighbor)
            {
              if (neighbor->active())
                neighbor->set_refinement_flag(Elem::REFINE);
              else
              {
                std::vector<Elem *> family_tree;
                neighbor->active_family_tree_by_neighbor(family_tree, elem);
                for (auto child_elem : family_tree)
                  child_elem->set_refinement_flag(Elem::REFINE);
              }
            }
          }
        }
      }
    }
  }
  MeshRefinement refinedmesh(*mesh);
  if (!_enable_neighbor_refinement)
    refinedmesh.face_level_mismatch_limit() = 0;
  refinedmesh.refine_elements();
  ref_step++;
  return recursive_refine(boundary_ids, mesh, refinement, max, ref_step);
}
