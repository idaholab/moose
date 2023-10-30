//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshPhysicsComponent.h"
#include "PhysicsBase.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshPhysicsComponent);

InputParameters
FileMeshPhysicsComponent::validParams()
{
  InputParameters params = FileMeshComponent::validParams();

  params.addClassDescription("Component with Physics objects active on it.");
  params.addParam<std::vector<PhysicsName>>("physics", "Physics object(s) active on the Component");

  params.addParam<std::vector<BoundaryName>>("incoming_boundaries",
                                             "Boundaries facing inwards for Junction purposes");
  params.addParam<std::vector<BoundaryName>>("outgoing_boundaries",
                                             "Boundaries facing outwards for Junction purposes");

  // We do not know which flow physics would be active so we cannot have the parameters of the
  // flow on the component. Parameters are not dynamic, they are known at compile time. Some
  // flow parameters could be nice to have on the component, such as in order the boundary
  // condition specifications, the heat source, etc. You quickly start to want all the
  // parameters of the Physics onto the component, which defeats the purpose of trying to factor
  // out Physics. Instead we should rely on:
  // - junctions to create boundary conditions

  // Having the flow parameters on the component would also undesirably lead to the following:
  // - having nearly a full input file on every flow component
  // - having to create a new component every time we have a new Physics

  // If you want to do that anyway, look at FileMeshWCNSFVFlowComponent

  return params;
}

FileMeshPhysicsComponent::FileMeshPhysicsComponent(const InputParameters & parameters)
  : FileMeshComponent(parameters)
{
}

void
FileMeshPhysicsComponent::init()
{
  FileMeshComponent::init();

  // Before this point, we did not have a problem, so we could not retrieve the physics
  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _physics.push_back(getMooseApp().actionWarehouse().getPhysics<PhysicsBase>(physics_name));

  for (auto physics : _physics)
    physics->addBlocks(getSubdomainNames());
}

void
FileMeshPhysicsComponent::setupMesh()
{
  FileMeshComponent::setupMesh();

  // Add connections for planned inlet and outlet
  const auto inlet_boundaries = getParam<std::vector<BoundaryName>>("incoming_boundaries");
  const auto outlet_boundaries = getParam<std::vector<BoundaryName>>("outgoing_boundaries");

  // For now we only need the boundary ids. The need for more Connection information may increase as
  // we add more types of junction techniques
  for (const auto i : index_range(inlet_boundaries))
    _connections[FileMeshComponentConnection::EEndType::IN].push_back(
        Connection(constMesh().getBoundaryID(inlet_boundaries[i])));
  for (const auto i : index_range(outlet_boundaries))
    _connections[FileMeshComponentConnection::EEndType::OUT].push_back(
        Connection(constMesh().getBoundaryID(outlet_boundaries[i])));
}

void
FileMeshPhysicsComponent::warnParamPassed(const InputParameters & warn_params,
                                          const InputParameters & allowed_params) const
{
  const auto warn_list = warn_params.getParametersList();
  const auto allow_list = allowed_params.getParametersList();
  for (const auto & param : parameters().getParametersList())
    if (warn_list.count(param) && isParamSetByUser(param) && !allow_list.count(param))
      mooseWarning("Parameter '" + param + "' has been passed but will be ignored");
}
