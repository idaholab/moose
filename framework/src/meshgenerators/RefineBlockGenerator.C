//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RefineBlockGenerator.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_refinement.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", RefineBlockGenerator);

InputParameters
RefineBlockGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which refines one or more blocks in an existing mesh");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to refine");
  params.addRequiredParam<std::vector<SubdomainName>>("block", "The list of blocks to be refined");
  params.addRequiredParam<std::vector<unsigned int>>(
      "refinement",
      "The amount of times to refine each block, corresponding to their index in 'block'");
  params.addParam<bool>(
      "enable_neighbor_refinement",
      true,
      "Toggles whether neighboring level one elements should be refined or not. Defaults to true");

  return params;
}

RefineBlockGenerator::RefineBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _block(getParam<std::vector<SubdomainName>>("block")),
    _refinement(getParam<std::vector<unsigned int>>("refinement")),
    _enable_neighbor_refinement(getParam<bool>("enable_neighbor_refinement"))
{
}

std::unique_ptr<MeshBase>
RefineBlockGenerator::generate()
{
  // Get the list of block ids from the block names
  const auto block_ids =
      MooseMeshUtils::getSubdomainIDs(*_input, getParam<std::vector<SubdomainName>>("block"));

  // Check that the block ids/names exist in the mesh
  std::set<SubdomainID> mesh_blocks;
  _input->subdomain_ids(mesh_blocks);

  for (std::size_t i = 0; i < block_ids.size(); ++i)
    if (!MooseMeshUtils::hasSubdomainID(*_input, block_ids[i]))
      paramError("block",
                 "The block '",
                 getParam<std::vector<SubdomainName>>("block")[i],
                 "' was not found within the mesh");

  std::unique_ptr<MeshBase> mesh = std::move(_input);
  int max = *std::max_element(_refinement.begin(), _refinement.end());

  if (max > 0 && !mesh->is_replicated() && !mesh->is_prepared())
    // refinement requires that (or at least it asserts that) the mesh is either replicated or
    // prepared
    mesh->prepare_for_use();

  return recursive_refine(block_ids, mesh, _refinement, max);
}

std::unique_ptr<MeshBase>
RefineBlockGenerator::recursive_refine(std::vector<subdomain_id_type> block_ids,
                                       std::unique_ptr<MeshBase> & mesh,
                                       std::vector<unsigned int> refinement,
                                       unsigned int max,
                                       unsigned int ref_step)
{
  if (ref_step == max)
    return dynamic_pointer_cast<MeshBase>(mesh);
  for (std::size_t i = 0; i < block_ids.size(); i++)
  {
    if (refinement[i] > 0 && refinement[i] > ref_step)
    {
      for (const auto & elem : mesh->active_subdomain_elements_ptr_range(block_ids[i]))
        elem->set_refinement_flag(Elem::REFINE);
    }
  }
  MeshRefinement refinedmesh(*mesh);
  if (!_enable_neighbor_refinement)
    refinedmesh.face_level_mismatch_limit() = 0;
  refinedmesh.refine_elements();

  ref_step++;
  return recursive_refine(block_ids, mesh, refinement, max, ref_step);
}
