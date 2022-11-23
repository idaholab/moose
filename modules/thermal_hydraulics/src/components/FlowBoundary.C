//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowBoundary.h"
#include "FlowChannelBase.h"

InputParameters
FlowBoundary::validParams()
{
  InputParameters params = Component1DBoundary::validParams();
  return params;
}

FlowBoundary::FlowBoundary(const InputParameters & params)
  : Component1DBoundary(params), _flow_model_id(THM::FM_INVALID)
{
}

void
FlowBoundary::init()
{
  Component1DBoundary::init();

  if (_connections.size() == 1)
    if (hasComponentByName<FlowChannelBase>(_connected_component_name))
    {
      const FlowChannelBase & comp =
          getTHMProblem().getComponentByName<FlowChannelBase>(_connected_component_name);

      _fp_name = comp.getFluidPropertiesName();
      _flow_model_id = comp.getFlowModelID();
      _flow_model = comp.getFlowModel();
    }
}

void
FlowBoundary::check() const
{
  Component1DBoundary::check();

  checkComponentOfTypeExistsByName<FlowChannelBase>(_connected_component_name);
}

const UserObjectName &
FlowBoundary::getFluidPropertiesName() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _fp_name;
}
