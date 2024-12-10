//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMBlockedMassFlowRateAux.h"

registerMooseObject("SubChannelApp", SCMBlockedMassFlowRateAux);

InputParameters
SCMBlockedMassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes inlet mass flow rate BCs, from specified mass flux and "
                             "cross-sectional area and applies blocked inlet conditions");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addRequiredParam<Real>("unblocked_mass_flux",
                                "Specified mass flux for unblocked subchannels [kg/s-m^2]");
  params.addRequiredParam<Real>("blocked_mass_flux",
                                "Specified mass flux for blocked subchannels [kg/s-m^2]]");
  params.declareControllable("unblocked_mass_flux");
  params.declareControllable("blocked_mass_flux");
  params.addRequiredParam<std::vector<unsigned int>>("index_blockage",
                                                     "index of subchannels affected by blockage");
  return params;
}

SCMBlockedMassFlowRateAux::SCMBlockedMassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh)),
    _unblocked_mass_flux(getParam<Real>("unblocked_mass_flux")),
    _blocked_mass_flux(getParam<Real>("blocked_mass_flux")),
    _area(coupledValue("area")),
    _index_blockage(getParam<std::vector<unsigned int>>("index_blockage"))
{
}

Real
SCMBlockedMassFlowRateAux::computeValue()
{
  auto i = _subchannel_mesh.getSubchannelIndexFromPoint(*_current_node);

  for (const auto & index : _index_blockage)
  {
    if (i == index)
      return _blocked_mass_flux * _area[_qp];
  }

  return _unblocked_mass_flux * _area[_qp];
}
