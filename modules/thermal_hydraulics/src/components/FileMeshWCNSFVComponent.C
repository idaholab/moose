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
#include "WCNSFVScalarAdvectionPhysics.h"
#include "WCNSFVTurbulencePhysics.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshWCNSFVComponent);

InputParameters
FileMeshWCNSFVComponent::validParams()
{
  InputParameters params = FileMeshPhysicsComponent::validParams();

  params.addClassDescription("Component that creates a Physics object active on it.");

  // Physics are hardcoded
  params.suppressParameter<std::vector<PhysicsName>>("physics");

  // Choose which physics to turn on
  params.addParam<bool>("add_flow_equations", true, "Whether to add equations for mass & momentum");
  params.addParam<bool>("add_energy_equation", true, "Whether to add the fluid energy equation");
  params.addParam<bool>(
      "add_scalar_equations", false, "Whether to add the scalar advection equations");

  // Add parameters from the various physics we want active on the component
  params += WCNSFVFlowPhysics::validParams();
  params += WCNSFVHeatAdvectionPhysics::validParams();
  params += WCNSFVScalarAdvectionPhysics::validParams();
  params += WCNSFVTurbulencePhysics::validParams();

  return params;
}

FileMeshWCNSFVComponent::FileMeshWCNSFVComponent(const InputParameters & parameters)
  : FileMeshPhysicsComponent(parameters),
    _has_flow_physics(getParam<bool>("add_flow_equations")),
    _has_heat_physics(getParam<bool>("add_energy_equation")),
    _has_scalar_physics(getParam<bool>("add_scalar_equations"))
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
  if (_has_flow_physics)
  {
    InputParameters params = getMooseApp().getActionFactory().getValidParams("WCNSFVFlowPhysics");
    params.applyParameters(parameters());
    const PhysicsName physics_name = prefix() + "flow";
    addPhysics("WCNSFVFlowPhysics", physics_name, params);

    // Keep track of new physics
    _physics_names.push_back(physics_name);
    _flow_physics = getMooseApp().actionWarehouse().getPhysics<WCNSFVFlowPhysics>(physics_name);
    _physics.push_back(_flow_physics);
  }
  if (_has_heat_physics)
  {
    InputParameters params =
        getMooseApp().getActionFactory().getValidParams("WCNSFVHeatAdvectionPhysics");
    params.applyParameters(parameters());
    if (getParam<bool>("add_flow_equations"))
      params.set<PhysicsName>("coupled_flow_physics") = prefix() + "flow";
    const PhysicsName physics_name = prefix() + "energy";
    addPhysics("WCNSFVHeatAdvectionPhysics", physics_name, params);

    // Keep track of new physics
    _physics_names.push_back(physics_name);
    _energy_physics =
        getMooseApp().actionWarehouse().getPhysics<WCNSFVHeatAdvectionPhysics>(physics_name);
    _physics.push_back(_energy_physics);
  }
  if (_has_scalar_physics)
  {
    InputParameters params =
        getMooseApp().getActionFactory().getValidParams("WCNSFVHeatAdvectionPhysics");
    params.applyParameters(parameters());
    if (getParam<bool>("add_flow_equations"))
      params.set<PhysicsName>("coupled_flow_physics") = prefix() + "flow";
    const PhysicsName physics_name = prefix() + "scalar";
    addPhysics("WCNSFVScalarAdvectionPhysics", physics_name, params);

    // Keep track of new physics
    _physics_names.push_back(physics_name);
    _scalar_physics =
        getMooseApp().actionWarehouse().getPhysics<WCNSFVScalarAdvectionPhysics>(physics_name);
    _physics.push_back(_scalar_physics);
  }

  for (auto physics : _physics)
    // Add block restriction
    physics->addBlocks(getSubdomainNames());
}

void
FileMeshWCNSFVComponent::setupMesh()
{
  // Add connections for planned inlet and outlet
  const auto inlet_boundaries = getParam<std::vector<BoundaryName>>("inlet_boundaries");
  const auto outlet_boundaries = getParam<std::vector<BoundaryName>>("outlet_boundaries");

  // For now we only need the boundary ids. The need for more Connection information may increase as
  // we add more types of junction techniques
  for (const auto i : index_range(inlet_boundaries))
    _connections[FileMeshComponentConnection::EEndType::IN].push_back(
        Connection(constMesh().getBoundaryID(inlet_boundaries[i])));
  for (const auto i : index_range(outlet_boundaries))
    _connections[FileMeshComponentConnection::EEndType::OUT].push_back(
        Connection(constMesh().getBoundaryID(outlet_boundaries[i])));

  FileMeshComponent::setupMesh();
}

bool
FileMeshWCNSFVComponent::hasPhysics(const PhysicsName & phys_name) const
{
  if (phys_name == "flow")
    return hasFlowPhysics();
  else if (phys_name == "energy")
    return hasFluidEnergyPhysics();
  else if (phys_name == "scalar")
    return hasScalarAdvectionPhysics();
  else
    mooseError("Short Physics name '", phys_name, "' not recognized");
}

WCNSFVPhysicsBase *
FileMeshWCNSFVComponent::getPhysics(const PhysicsName & phys_name) const
{
  auto physics =
      getMooseApp().actionWarehouse().getPhysics<WCNSFVPhysicsBase>(prefix() + phys_name);
  if (!physics)
    mooseError("Physics " + prefix() + phys_name + " is not derived from WCNSFVPhysicsBase");
  return physics;
}

VariableName
FileMeshWCNSFVComponent::getVariableName(const std::string & default_name) const
{
  if (hasFlowPhysics())
    return _flow_physics->getFlowVariableName(default_name);
  else if (hasFluidEnergyPhysics())
    return _energy_physics->getFlowVariableName(default_name);
  else if (hasScalarAdvectionPhysics())
    return _scalar_physics->getFlowVariableName(default_name);
  else
    mooseError(
        "No Physics object to provide the true variable name for the default variable name: ",
        default_name);
}

void
FileMeshWCNSFVComponent::addPhysics(const std::string & action_name,
                                    const std::string & physics_name,
                                    InputParameters & params)
{
  // Add the Physics/Action to be created. All the tasks associated with this
  // action will be run
  auto action_factory = getMooseApp().getActionFactory();
  std::shared_ptr<Action> ptr = action_factory.create(action_name, physics_name, params);
  getMooseApp().actionWarehouse().addActionBlock(ptr);
}
