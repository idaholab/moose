//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsFlowBoundary.h"
#include "PhysicsFlowChannel.h"
#include "ActionWarehouse.h"
#include "ThermalHydraulicsFlowPhysics.h"

InputParameters
PhysicsFlowBoundary::validParams()
{
  InputParameters params = FlowBoundary::validParams();
  params.addRequiredParam<std::vector<PhysicsName>>("physics",
                                                    "Physics active on the flow boundary");
  return params;
}

PhysicsFlowBoundary::PhysicsFlowBoundary(const InputParameters & params)
  : FlowBoundary(params), _boundary_uo_name(genName(name(), "boundary_uo"))
{
}

void
PhysicsFlowBoundary::init()
{
  FlowBoundary::init();

  if (hasComponentByName<PhysicsFlowChannel>(_connected_component_name))
  {
    // This will likely be necessary
    // const PhysicsFlowChannel & comp =
    //    getTHMProblem().getComponentByName<PhysicsFlowChannel>(_connected_component_name);
    // for (const auto physics : comp.getPhysics())
    //   _connected_physics.push_back(physics);
  }

  if (isParamSetByUser("physics"))
    for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
      _th_physics.push_back(
          _app.actionWarehouse().getPhysics<ThermalHydraulicsFlowPhysics>(physics_name));
  // NOTE: we currently expect to error on non thermal-hydraulics physics.
  // This may be removed in the future
}

void
PhysicsFlowBoundary::check() const
{
  FlowBoundary::check();

  checkComponentOfTypeExistsByName<PhysicsFlowChannel>(_connected_component_name);
}
