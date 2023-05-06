//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryDeletionGenerator.h"

#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", BoundaryDeletionGenerator);

InputParameters
BoundaryDeletionGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which removes side sets");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryName>>("boundary_names",
                                                     "The boundaries to be deleted");

  return params;
}

BoundaryDeletionGenerator::BoundaryDeletionGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary_names"))
{
}

std::unique_ptr<MeshBase>
BoundaryDeletionGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  for (const auto & name : _boundary_names)
  {
    auto bid = MooseMeshUtils::getBoundaryID(name, *mesh);
    if (bid == BoundaryInfo::invalid_id)
      paramError("boundary_names", "The boundary '", name, "' was not found in the mesh");

    mesh->get_boundary_info().remove_id(bid);
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
