//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshFlowComponent.h"
#include "NavierStokesFlowPhysics.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshFlowComponent);

InputParameters
FileMeshFlowComponent::validParams()
{
  InputParameters params = FileMeshComponent::validParams();

  params.addClassDescription("Component with a Navier Stokes flow physics active on it.");
  params.addParam<std::vector<PhysicsName>>("physics", "Physics object(s) active on the Component");

  // We do not know which flow physics would be active so we cannot have the parameters of the flow
  // on the component. Parameters are not dynamic, they are known at compile time.
  // Some flow parameters could be nice to have on the component, such as in order the boundary
  // condition specifications, the heat source, etc. You quickly start to want all the parameters of
  // the Physics onto the component, which defeats the purpose of trying to factor out Physics.
  // Instead we should rely on:
  // - junctions to create boundary conditions

  // Having the flow parameters on the component would also undesirably lead to the following:
  // - having nearly a full input file on every flow component
  // - having to create a new component every time we have a new Physics
  return params;
}

FileMeshFlowComponent::FileMeshFlowComponent(const InputParameters & parameters)
  : FileMeshPhysicsComponent(parameters)
{
}

void
FileMeshFlowComponent::addRelationshipManagers(Moose::RelationshipManagerType /*input_rm_type*/)
{
  // TODO: We ll just add late relationship managers
  // At this point in the setup, we do not have a problem, so we cannot retrieve a Physics. We can
  // send the default ghosting for the physics, but that's it.
  addRelationshipManagersFromParameters(NavierStokesFlowPhysics::validParams());
}

void
FileMeshFlowComponent::init()
{
  FileMeshComponent::init();

  // Before this point, we did not have a Problem object, so we could not retrieve the physics
  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _physics.push_back(
        dynamic_cast<NavierStokesFlowPhysics *>(getProblem().getPhysics(physics_name)));
  mooseAssert(std::find(_physics.begin(), _physics.end(), nullptr) == _physics.end(),
              "One of the Physics is not a Navier Stokes flow physics");

  for (auto physics : _physics)
    // Add block restriction
    physics->addBlocks(getSubdomainNames());
}
