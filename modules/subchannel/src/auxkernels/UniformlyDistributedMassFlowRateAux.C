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
    _subchannel_mesh(dynamic_cast<SubChannelMeshBase &>(_mesh))
{
}

Real
UniformlyDistributedMassFlowRateAux::computeValue()
{
  unsigned int n_ch = _subchannel_mesh.getNumOfChannels();
  return _mass_flow / n_ch;
}