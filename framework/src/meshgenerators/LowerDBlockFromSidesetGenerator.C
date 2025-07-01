//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LowerDBlockFromSidesetGenerator.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/compare_elems_by_level.h"
#include "libmesh/mesh_communication.h"

#include "timpi/parallel_sync.h"

#include <set>
#include <typeinfo>

registerMooseObject("MooseApp", LowerDBlockFromSidesetGenerator);

InputParameters
LowerDBlockFromSidesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<SubdomainID>("new_block_id", "The lower dimensional block id to create");
  params.addParam<SubdomainName>("new_block_name",
                                 "The lower dimensional block name to create (optional)");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "sidesets", "The sidesets from which to create the new block");

  params.addClassDescription("Adds lower dimensional elements on the specified sidesets.");

  return params;
}

LowerDBlockFromSidesetGenerator::LowerDBlockFromSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _sideset_names(getParam<std::vector<BoundaryName>>("sidesets"))
{
}

std::unique_ptr<MeshBase>
LowerDBlockFromSidesetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Generate a new block id if one isn't supplied.
  SubdomainID new_block_id = isParamValid("new_block_id")
                                 ? getParam<SubdomainID>("new_block_id")
                                 : MooseMeshUtils::getNextFreeSubdomainID(*mesh);
  try
  {
    MooseMeshUtils::createSubdomainFromSidesets(mesh,
                                                _sideset_names,
                                                new_block_id,
                                                isParamValid("new_block_name")
                                                    ? getParam<SubdomainName>("new_block_name")
                                                    : SubdomainName(),
                                                type());
  }
  catch (MooseException & e)
  {
    paramError("sidesets", e.what());
  }

  return mesh;
}
