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
  params.addParam<bool>("add_flow_equations", "Whether to add equations for mass & momentum");
  params.addParam<bool>("add_energy_equation", "Whether to add the fluid energy equation");
  params.addParam<bool>("add_scalar_equations", "Whether to add the scalar advection equations");

  // Add parameters from the various physics we want active on the component
  // params += WCNSFVFlowPhysics::validParams();
  // params += WCNSFVHeatAdvectionPhysics::validParams();
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
  addRelationshipManagersFromParameters(NavierStokesFlowPhysics::validParams());
}

void
FileMeshWCNSFVComponent::init()
{
  FileMeshComponent::init();

  // It s either the const_cast or loosing the ability to control Physics parameters
  // from the component's parameters. We would still be able to use Controls directly on the
  // objects created by the Physics
  InputParameters & nonconst_params = const_cast<InputParameters &>(parameters());

  // Before this point, we did not have a Problem object, so we could not add the Physics
  if (true) // getParam<bool>("add_flow_equations"))
  {
    getProblem().addPhysics("WCNSFVFlowPhysics", prefix() + "_flow", nonconst_params);
    _physics_names.push_back(prefix() + "_flow");
  }
  if (getParam<bool>("add_energy_equation"))
  {
    getProblem().addPhysics("WCNSFVHeatAdvectionPhysics", prefix() + "_energy", nonconst_params);
    _physics_names.push_back(prefix() + "_energy");
  }
  // if (getParam<bool>("add_scalar_equations"))
  // {
  //   getProblem().addPhysics("WCNSFVScalarAdvectionPhysics", prefix() + "_scalar",
  //   nonconst_params); _physics_names.push_back(prefix() + "_scalar");
  // }

  // Keep a handle on the Physics
  for (const auto & physics_name : _physics_names)
    _physics.push_back(dynamic_cast<WCNSFVPhysicsBase *>(getProblem().getPhysics(physics_name)));

  for (auto physics : _physics)
    // Add block restriction
    physics->addBlocks(getSubdomainNames());
}
