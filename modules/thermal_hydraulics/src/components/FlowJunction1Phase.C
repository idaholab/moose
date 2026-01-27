//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowJunction1Phase.h"
#include "FlowChannel1Phase.h"

InputParameters
FlowJunction1Phase::validParams()
{
  InputParameters params = FlowJunction::validParams();
  return params;
}

FlowJunction1Phase::FlowJunction1Phase(const InputParameters & params) : FlowJunction(params) {}

void
FlowJunction1Phase::init()
{
  FlowJunction::init();

  for (const auto i : index_range(_connections))
  {
    const std::string comp_name = _connections[i]._component_name;
    if (hasComponentByName<FlowChannel1Phase>(comp_name))
    {
      const FlowChannel1Phase & comp =
          getTHMProblem().getComponentByName<FlowChannel1Phase>(comp_name);

      _numerical_flux_names.push_back(comp.getNumericalFluxUserObjectName());

      if (i == 0)
        _passives_names = comp.getParam<std::vector<VariableName>>("passives_names");
      else if (comp.getParam<std::vector<VariableName>>("passives_names") != _passives_names)
        logError("All connected channels must have the same 'passives_names' parameter.");
    }
  }
}

void
FlowJunction1Phase::check() const
{
  FlowJunction::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<FlowChannel1Phase>(comp_name);

  if (_passives_names.size() > 0 && !supportsPassiveTransport())
    logError("This Component type does not support passive transport.");
}
