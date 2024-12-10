//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMFlatMassFlowRateAux.h"

registerMooseObject("SubChannelApp", SCMFlatMassFlowRateAux);

InputParameters
SCMFlatMassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<Real>("mass_flow", "Specified total mass flow at the inlet [kg/s]");
  params.addClassDescription("Computes a uniform mass flow rate at the inlet");
  return params;
}

SCMFlatMassFlowRateAux::SCMFlatMassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mass_flow(getParam<Real>("mass_flow")),
    _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh))
{
}

Real
SCMFlatMassFlowRateAux::computeValue()
{
  unsigned int n_ch = _subchannel_mesh.getNumOfChannels();
  return _mass_flow / n_ch;
}
