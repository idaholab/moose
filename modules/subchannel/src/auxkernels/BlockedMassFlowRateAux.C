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
  params.addClassDescription("Computes mass float rate from specified mass flux and "
                             "cross-sectional area and applies blocked inlet conditions");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addRequiredParam<Real>("mass_flux", "Specified mass flux [kg/s-m^2]");
  params.declareControllable("mass_flux");
  params.addParam<std::vector<unsigned int>>("index_blockage",
                                             std::vector<unsigned int>({1000}),
                                             "index of subchannels affected by blockage");
  return params;
}

BlockedMassFlowRateAux::BlockedMassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh)),
    _mass_flux(getParam<Real>("mass_flux")),
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
      return 0.001; // TO:D0 make it a user parameter?
  }

  return _mass_flux * _area[_qp];
}
