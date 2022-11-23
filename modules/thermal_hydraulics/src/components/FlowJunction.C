//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowJunction.h"
#include "FlowChannelBase.h"

InputParameters
FlowJunction::validParams()
{
  InputParameters params = Component1DJunction::validParams();
  params.addPrivateParam<std::string>("component_type", "flow_junction");
  return params;
}

FlowJunction::FlowJunction(const InputParameters & params)
  : Component1DJunction(params),

    _junction_uo_name(genName(name(), "junction_uo"))
{
}

void
FlowJunction::init()
{
  Component1DJunction::init();

  if (_connections.size() > 0)
  {
    std::vector<UserObjectName> fp_names;
    std::vector<THM::FlowModelID> flow_model_ids;
    for (const auto & connection : _connections)
    {
      const std::string comp_name = connection._component_name;
      if (hasComponentByName<FlowChannelBase>(comp_name))
      {
        const FlowChannelBase & comp =
            getTHMProblem().getComponentByName<FlowChannelBase>(comp_name);

        fp_names.push_back(comp.getFluidPropertiesName());
        flow_model_ids.push_back(comp.getFlowModelID());
      }
    }

    if (fp_names.size() > 0)
    {
      checkAllConnectionsHaveSame<UserObjectName>(fp_names, "fluid properties object");
      _fp_name = fp_names[0];

      checkAllConnectionsHaveSame<THM::FlowModelID>(flow_model_ids, "flow model ID");
      _flow_model_id = flow_model_ids[0];

      if (hasComponentByName<FlowChannelBase>(_connections[0]._component_name))
      {
        const FlowChannelBase & flow_channel =
            getComponentByName<FlowChannelBase>(_connections[0]._component_name);
        _flow_model = flow_channel.getFlowModel();
      }
    }
  }
}

void
FlowJunction::check() const
{
  Component1DJunction::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<FlowChannelBase>(comp_name);
}

const UserObjectName &
FlowJunction::getFluidPropertiesName() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _fp_name;
}
