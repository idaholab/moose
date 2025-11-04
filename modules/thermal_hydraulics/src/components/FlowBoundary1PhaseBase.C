//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowBoundary1PhaseBase.h"
#include "FlowChannel1PhaseBase.h"

InputParameters
FlowBoundary1PhaseBase::validParams()
{
  InputParameters params = FlowBoundary::validParams();
  return params;
}

FlowBoundary1PhaseBase::FlowBoundary1PhaseBase(const InputParameters & params)
  : FlowBoundary(params), _boundary_uo_name(genName(name(), "boundary_uo"))
{
}

void
FlowBoundary1PhaseBase::init()
{
  FlowBoundary::init();

  if (hasComponentByName<FlowChannel1PhaseBase>(_connected_component_name))
  {
    const auto & flow_channel =
        getTHMProblem().getComponentByName<FlowChannel1PhaseBase>(_connected_component_name);
    _numerical_flux_name = flow_channel.getNumericalFluxUserObjectName();
  }
}
