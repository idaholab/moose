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

registerMooseObject("MooseApp", RefineBlockGenerator);

defineLegacyParams(RefineBlockGenerator);

InputParameters
RefineBlockGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which removes elements from the specified subdomains");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to modify");
  params.addRequiredParam<std::vector<SubdomainName>>("block", "The list of blocks to be modified");
  params.addRequiredParam<std::vector<int>>("refinement", "foo");
  
  return params;
}

RefineBlockGenerator::RefineBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
     _input(getMesh("input")),
    _block(getParam<std::vector<SubdomainName>>("block")),
    _refinement(getParam<std::vector<int>>("refinement"))
  {}
std::unique_ptr<MeshBase>
RefineBlockGenerator::generate()
{

  std::unique_ptr<MeshBase> mesh = std::move(_input);

    // Get the list of block ids from the block names
  const auto block_ids =
        MooseMeshUtils::getSubdomainIDs(*_input, getParam<std::vector<SubdomainName>>("block"));

  // Check that the block ids/names exist in the mesh
  std::set<SubdomainID> mesh_blocks;
  _input->subdomain_ids(mesh_blocks);

  for (std::size_t i = 0; i < block_ids.size(); ++i)
    if (block_ids[i] == Moose::INVALID_BLOCK_ID || !mesh_blocks.count(block_ids[i]))
    {
      if (isParamValid("block"))
        paramError("block",
                   "The block '",
                   getParam<std::vector<SubdomainName>>("block")[i],
                   "' was not found within the mesh");
      else
        paramError("block_id",
                   "The block '",
                   getParam<SubdomainID>("block_id"),
                   "' was not found within the mesh");
    }

  for (const auto & elem : mesh->active_element_ptr_range())
    for (auto i = 0; i < block_ids.size(); ++i)
      elem->set_refinement_flag(Elem::REFINE);

  MeshRefinement refinedmesh(*mesh);
  refinedmesh.refine_elements();

  return dynamic_pointer_cast<MeshBase>(mesh);
}

