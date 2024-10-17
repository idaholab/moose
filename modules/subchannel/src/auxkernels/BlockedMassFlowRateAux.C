/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "BlockedMassFlowRateAux.h"

registerMooseObject("SubChannelApp", BlockedMassFlowRateAux);

InputParameters
BlockedMassFlowRateAux::validParams()
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

BlockedMassFlowRateAux::BlockedMassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh)),
    _unblocked_mass_flux(getParam<Real>("unblocked_mass_flux")),
    _blocked_mass_flux(getParam<Real>("blocked_mass_flux")),
    _area(coupledValue("area")),
    _index_blockage(getParam<std::vector<unsigned int>>("index_blockage"))
{
}

Real
BlockedMassFlowRateAux::computeValue()
{
  auto i = _subchannel_mesh.getSubchannelIndexFromPoint(*_current_node);

  for (const auto & index : _index_blockage)
  {
    if (i == index)
      return _blocked_mass_flux * _area[_qp];
  }

  return _unblocked_mass_flux * _area[_qp];
}
