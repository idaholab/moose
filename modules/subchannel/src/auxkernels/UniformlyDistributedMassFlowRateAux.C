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

#include "UniformlyDistributedMassFlowRateAux.h"

registerMooseObject("SubChannelApp", UniformlyDistributedMassFlowRateAux);

InputParameters
UniformlyDistributedMassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<Real>("mass_flow", "Specified total mass flow at the inlet [kg/s]");
  params.addClassDescription("Computes a uniform mass flow rate at the inlet");
  return params;
}

UniformlyDistributedMassFlowRateAux::UniformlyDistributedMassFlowRateAux(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _mass_flow(getParam<Real>("mass_flow")),
    _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh))
{
}

Real
UniformlyDistributedMassFlowRateAux::computeValue()
{
  unsigned int n_ch = _subchannel_mesh.getNumOfChannels();
  return _mass_flow / n_ch;
}
