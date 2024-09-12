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

registerActionComponent("MooseApp", MeshGeneratorComponent);
registerMooseAction("MooseApp", MeshGeneratorComponent, "add_mesh_generator");
registerMooseAction("MooseApp", MeshGeneratorComponent, "init_component_physics");

InputParameters
MeshGeneratorComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += PhysicsComponentBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("mesh_generator", "Mesh generator providing the mesh");
  MooseEnum mesh_generator_type("saved_mesh final_generator");
  params.addRequiredParam<MooseEnum>("mesh_generator_type",
                                     mesh_generator_type,
                                     "Whether the mesh generator providing the mesh is the final "
                                     "mesh generator, or simply provides a 'saved_mesh'");
  params.addParam<std::string>("saved_mesh_name", "Name used to generate the mesh");
  params.addClassDescription("Component with a mesh coming from a mesh generator.");

  return params;
}

MeshGeneratorComponent::MeshGeneratorComponent(const InputParameters & params)
  : ActionComponent(params),
    PhysicsComponentBase(params),
    _mesh_generator_type(getParam<MooseEnum>("mesh_generator_type"))
{
  if (_mesh_generator_type == "saved_mesh" && !isParamValid("saved_mesh_name"))
    paramError("saved_mesh_name",
               "The name of the saved mesh must be provided if using a mesh generator that saves "
               "the mesh for the component to use");

  addRequiredTask("add_mesh_generator");
}

void
MeshGeneratorComponent::addMeshGenerators()
{
  // The mesh generator will end up as an input to the final combiner generator
  _mg_names.push_back(getParam<MeshGeneratorName>("mesh_generator"));
}

void
MeshGeneratorComponent::setupComponent()
{
  const auto saved_mesh =
      (_mesh_generator_type == "saved_mesh")
          ? _app.getMeshGeneratorSystem().getSavedMesh(getParam<std::string>("saved_mesh_name"))
          : nullptr;
  const auto component_mesh = saved_mesh ? saved_mesh.get() : _mesh->getMeshPtr();
  mooseAssert(component_mesh, "Should have a mesh");

  // Get list of blocks from the saved mesh
  std::set<subdomain_id_type> blocks;
  component_mesh->subdomain_ids(blocks);
  for (const auto bid : blocks)
    _blocks.push_back(component_mesh->subdomain_name(bid));
}
