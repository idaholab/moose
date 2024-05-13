//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MeshGeneratorComponent.h"
#include "RotationMatrix.h"

registerMooseAction("MooseApp", MeshGeneratorComponent, "add_mesh_generator");
registerMooseAction("MooseApp", MeshGeneratorComponent, "add_positions");
registerMooseAction("MooseApp", MeshGeneratorComponent, "init_physics");

InputParameters
MeshGeneratorComponent::validParams()
{
  InputParameters params = ComponentAction::validParams();
  params.addClassDescription("Component with a mesh coming from a mesh generator.");
  params.addRequiredParam<MeshGeneratorName>("mesh_generator", "Mesh generator providing the mesh");
  params.addRequiredParam<std::string>("saved_mesh_name", "Name used to generate the mesh");
  params.addParam<std::vector<PhysicsName>>(
      "physics", {}, "Physics object(s) active on the Component");

  return params;
}

MeshGeneratorComponent::MeshGeneratorComponent(const InputParameters & params)
  : ComponentAction(params)
{
}

void
MeshGeneratorComponent::addMeshGenerators()
{
  // The mesh generator will end up as an input to the final combiner generator
  _mg_name = getParam<MeshGeneratorName>("mesh_generator");
}

void
MeshGeneratorComponent::addPhysics()
{
  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _physics.push_back(getMooseApp().actionWarehouse().getPhysics<PhysicsBase>(physics_name));

  // Get list of blocks from the saved mesh
  const auto saved_mesh =
      _app.getMeshGeneratorSystem().getSavedMesh(getParam<std::string>("saved_mesh_name"));
  std::set<subdomain_id_type> blocks;
  saved_mesh->subdomain_ids(blocks);
  std::vector<SubdomainName> blocks_vec;
  for (const auto bid : blocks)
    blocks_vec.push_back(saved_mesh->subdomain_name(bid));

  for (auto physics : _physics)
  {
    _console << "Adding Physics " << physics->name() << " on component " << name() << " on blocks "
             << Moose::stringify(blocks_vec) << std::endl;
    physics->addBlocks(blocks_vec);
  }
}
