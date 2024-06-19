//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SavedMeshComponent.h"
#include "RotationMatrix.h"

registerMooseAction("MooseApp", SavedMeshComponent, "add_mesh_generator");
registerMooseAction("MooseApp", SavedMeshComponent, "add_positions");
registerMooseAction("MooseApp", SavedMeshComponent, "init_physics");

InputParameters
SavedMeshComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += PhysicsComponentHelper::validParams();
  params.addRequiredParam<MeshGeneratorName>("mesh_generator", "Mesh generator providing the mesh");
  params.addRequiredParam<std::string>("saved_mesh_name", "Name used to generate the mesh");
  params.addClassDescription("Component with a mesh coming from a mesh generator.");

  return params;
}

SavedMeshComponent::SavedMeshComponent(const InputParameters & params)
  : ActionComponent(params), PhysicsComponentHelper(params)
{
}

void
SavedMeshComponent::addMeshGenerators()
{
  // The mesh generator will end up as an input to the final combiner generator
  _mg_name = getParam<MeshGeneratorName>("mesh_generator");
}

void
SavedMeshComponent::setupComponent()
{
  // Get list of blocks from the saved mesh
  const auto saved_mesh =
      _app.getMeshGeneratorSystem().getSavedMesh(getParam<std::string>("saved_mesh_name"));
  std::set<subdomain_id_type> blocks;
  saved_mesh->subdomain_ids(blocks);
  std::vector<SubdomainName> blocks_vec;
  for (const auto bid : blocks)
    blocks_vec.push_back(saved_mesh->subdomain_name(bid));
}
