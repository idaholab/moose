//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelAddInitialConditionsAction.h"
#include "ActionFactory.h"
#include "FEProblemBase.h"
#include "QuadSubChannelMesh.h"
#include "SubChannelApp.h"
#include "SubChannelMesh.h"
#include "TriSubChannelMesh.h"

registerMooseAction("SubChannelApp", SubChannelAddInitialConditionsAction, "sch:add_default_ic");

InputParameters
SubChannelAddInitialConditionsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Adds the default initial conditions for subchannel geometry variables");
  return params;
}

SubChannelAddInitialConditionsAction::SubChannelAddInitialConditionsAction(
    const InputParameters & parameters)
  : Action(parameters)
{
}

void
SubChannelAddInitialConditionsAction::act()
{
  const auto * const quad_mesh = dynamic_cast<const QuadSubChannelMesh *>(_mesh.get());
  const auto * const tri_mesh = dynamic_cast<const TriSubChannelMesh *>(_mesh.get());
  const bool is_quad = quad_mesh != nullptr;
  const bool is_tri = tri_mesh != nullptr;
  const SubChannelMesh * subchannel_mesh = nullptr;

  if (is_quad)
  {
    subchannel_mesh = quad_mesh;
    addInitialCondition("SCMQuadFlowAreaIC", "subchannel_S_IC", SubChannelApp::SURFACE_AREA);
    addInitialCondition(
        "SCMQuadWettedPerimIC", "subchannel_w_perim_IC", SubChannelApp::WETTED_PERIMETER);
  }
  else if (is_tri)
  {
    subchannel_mesh = tri_mesh;
    addInitialCondition("SCMTriFlowAreaIC", "subchannel_S_IC", SubChannelApp::SURFACE_AREA);
    addInitialCondition(
        "SCMTriWettedPerimIC", "subchannel_w_perim_IC", SubChannelApp::WETTED_PERIMETER);
  }
  else
    return;

  if (subchannel_mesh->pinMeshExist() && !hasInitialCondition(SubChannelApp::PIN_DIAMETER))
  {
    auto params = _factory.getValidParams("ConstantIC");
    params.set<VariableName>("variable") = SubChannelApp::PIN_DIAMETER;
    params.set<Real>("value") = subchannel_mesh->getPinDiameter();
    _problem->addInitialCondition("ConstantIC", "subchannel_Dpin_IC", params);
  }
}

void
SubChannelAddInitialConditionsAction::addInitialCondition(const std::string & type,
                                                          const std::string & name,
                                                          const VariableName & var_name)
{
  if (hasInitialCondition(var_name))
    return;

  auto params = _factory.getValidParams(type);
  params.set<VariableName>("variable") = var_name;
  _problem->addInitialCondition(type, name, params);
}

bool
SubChannelAddInitialConditionsAction::hasInitialCondition(const VariableName & var_name) const
{
  return _problem->getInitialConditionWarehouse().hasObjectsForVariable(var_name, /*tid=*/0);
}
