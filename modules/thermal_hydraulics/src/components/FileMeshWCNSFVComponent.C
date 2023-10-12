//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshWCNSFVComponent.h"
#include "WCNSFVFlowPhysics.h"
#include "WCNSFVHeatAdvectionPhysics.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshWCNSFVComponent);

InputParameters
FileMeshWCNSFVComponent::validParams()
{
  InputParameters params = FileMeshComponent::validParams();

  params.addClassDescription("Component that creates a Physics object active on it.");

  // Choose which physics to turn on
  params.addParam<bool>("add_flow_equations", true, "Whether to add equations for mass & momentum");
  params.addParam<bool>("add_energy_equation", true, "Whether to add the fluid energy equation");
  params.addParam<bool>(
      "add_scalar_equations", false, "Whether to add the scalar advection equations");

  // Add parameters from the various physics we want active on the component
  params += WCNSFVFlowPhysics::validParams();
  params += WCNSFVHeatAdvectionPhysics::validParams();
  // params += WCNSFVScalarAdvectionPhysics::validParams;
  // params += WCNSFVTurbulencePhysics::validParams;

  return params;
}

FileMeshWCNSFVComponent::FileMeshWCNSFVComponent(const InputParameters & parameters)
  : FileMeshComponent(parameters)
{
}

void
FileMeshWCNSFVComponent::addRelationshipManagers(Moose::RelationshipManagerType /*input_rm_type*/)
{
  // At this point in the setup, we do not have a problem, so we cannot retrieve a Physics. We can
  // send the default ghosting for the physics, but that's it
  addRelationshipManagersFromParameters(NavierStokesFlowPhysicsBase::validParams());
}

void
FileMeshWCNSFVComponent::init()
{
  FileMeshComponent::init();

  // Before this point, we did not have a Problem object, so we could not add the Physics
  if (getParam<bool>("add_flow_equations"))
  {
    InputParameters params = getFactory().getValidParams("WCNSFVFlowPhysics");
    params.applyParameters(parameters());
    getProblem().addPhysics("WCNSFVFlowPhysics", prefix() + "flow", params);
    _physics_names.push_back(prefix() + "flow");
  }
  if (getParam<bool>("add_energy_equation"))
  {
    InputParameters params = getFactory().getValidParams("WCNSFVHeatAdvectionPhysics");
    params.applyParameters(parameters());
    if (getParam<bool>("add_flow_equations"))
      params.set<PhysicsName>("coupled_flow_physics") = prefix() + "flow";
    getProblem().addPhysics("WCNSFVHeatAdvectionPhysics", prefix() + "energy", params);
    _physics_names.push_back(prefix() + "energy");
  }
  if (getParam<bool>("add_scalar_equations"))
  {
    InputParameters params = getFactory().getValidParams("WCNSFVHeatAdvectionPhysics");
    params.applyParameters(parameters());
    if (getParam<bool>("add_flow_equations"))
      params.set<PhysicsName>("coupled_flow_physics") = prefix() + "flow";
    getProblem().addPhysics("WCNSFVScalarAdvectionPhysics", prefix() + "scalar", params);
    _physics_names.push_back(prefix() + "scalar");
  }

  // Keep a handle on the Physics
  for (const auto & physics_name : _physics_names)
    _physics.push_back(dynamic_cast<WCNSFVPhysicsBase *>(getProblem().getPhysics(physics_name)));

  for (auto physics : _physics)
    // Add block restriction
    physics->addBlocks(getSubdomainNames());
}

void
FileMeshWCNSFVComponent::addVariables()
{
  for (auto physics : _physics)
    physics->addNonlinearVariables();
}

void
FileMeshWCNSFVComponent::addMooseObjects()
{
  // It should not matter that we dont add objects by categories
  for (auto physics : _physics)
  {
    // Rhie Chow user object must be added early
    physics->addUserObjects();
    physics->addInitialConditions();
    physics->addFVKernels();
    physics->addFVBCs();
    physics->addMaterials();
    physics->addFunctorMaterials();
    physics->addPostprocessors();
  }
}
