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
  MooseEnum boundary_side("primary secondary both", "both");
  params.addClassDescription("Mesh generator which refines one or more sidesets");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to modify");
  params.addRequiredParam<std::vector<BoundaryName>>("boundaries", "The list of boundaries to be modified");
  params.addRequiredParam<std::vector<int>>(
      "refinement",
      "The amount of times to refine each block, corresponding to their index in 'block'");
  params.addParam<bool>(
      "enable_neighbor_refinement",
      true,
      "Toggles whether neighboring level one elements should be refined or not. Defaults to true");
  params.addParam<std::vector<MooseEnum>>("boundary_side", "  ");

  return params;
}

RefineSidesetGenerator::RefineSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundaries(getParam<std::vector<BoundaryName>>("boundaries")),
    _refinement(getParam<std::vector<int>>("refinement")),
    _enable_neighbor_refinement(getParam<bool>("enable_neighbor_refinement")),
    _boundary_side(getParam<std::vector<MooseEnum>>("boundary_side"))
{
}
std::unique_ptr<MeshBase>
RefineSidesetGenerator::generate()
{

  // Get the list of boundary ids from the boundary names
  const auto boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*_input, getParam<std::vector<BoundaryName>>("boundary"), true);

  // Check that the block ids/names exist in the mesh
  /*
  std::set<SubdomainID> mesh_boundaries;
  _input->subdomain_ids(mesh_blocks);

  for (std::size_t i = 0; i < boundary_ids.size(); ++i)
    if (boundary_ids[i] == Moose::INVALID_BLOCK_ID || !mesh_boundaries.count(boundary_ids[i]))
    {
      if (isParamValid("_boundaries"))
        paramError("boundaries",
                   "The boundary '",
                   getParam<std::vector<BoundaryName>>("boundaries")[i],
                   "' was not found within the mesh");
    }
  */
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  int max = *std::max_element(_refinement.begin(), _refinement.end());

  return recursive_refine(boundary_ids, _boundary_side, mesh, _refinement, max);
}

std::unique_ptr<MeshBase>
RefineSidesetGenerator::recursive_refine(std::vector<boundary_id_type> boundary_ids,
                                    std::vector<MooseEnum> boundary_side,
                                    std::unique_ptr<MeshBase> & mesh,
                                    std::vector<int> refinement,
                                    int max,
                                    int ref_step)
{

  if (ref_step == max)
    return dynamic_pointer_cast<MeshBase>(mesh);
  for (std::size_t i = 0; i < boundary_ids.size(); i++)
  {
    if (refinement[i] > 0 && refinement[i] > ref_step)
    {
      BoundaryInfo boundInfo = mesh->get_boundary_info();
      std::vector< std::tuple< dof_id_type, unsigned short int, boundary_id_type > > sideList = boundInfo.build_active_side_list();
      for (std::tuple<dof_id_type, unsigned short int, boundary_id_type> tuple : sideList){
        if (std::get<2>(tuple) == boundary_ids[i]){
          //if (_boundary_side == "primary" || _boundary_side == "both"){
          std::unique_ptr< Elem > elem = Elem::build_side_ptr(std::get<1>(tuple));
          elem->set_refinement_flag(Elem::REFINE);
          }
          /*
          if (_boundary_side == "secondary" || _boundary_side == "both")
            //std::get<0>(elem).neighbor_ptr(i)->set_refinement_flag(Elem::REFINE);
            std::get<0>(elem);
        }*/
      }
    }
  }
  MeshRefinement refinedmesh(*mesh);
  if (!_enable_neighbor_refinement)
    refinedmesh.face_level_mismatch_limit() = 0;
  refinedmesh.refine_elements();

  ref_step++;
  return recursive_refine(boundary_ids, boundary_side, mesh, refinement, max, ref_step);
}
