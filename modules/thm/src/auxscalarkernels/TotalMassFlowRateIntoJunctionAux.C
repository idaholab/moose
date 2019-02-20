#include "TotalMassFlowRateIntoJunctionAux.h"

registerMooseObject("THMApp", TotalMassFlowRateIntoJunctionAux);

template <>
InputParameters
validParams<TotalMassFlowRateIntoJunctionAux>()
{
  InputParameters params = validParams<AuxNodalScalarKernel>();
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addRequiredParam<std::vector<Real>>("normals",
                                             "Component of outward normals along 1-D direction");

  return params;
}

TotalMassFlowRateIntoJunctionAux::TotalMassFlowRateIntoJunctionAux(
    const InputParameters & parameters)
  : AuxNodalScalarKernel(parameters),
    _rhouA(coupledValue("rhouA")),
    _normals(getParam<std::vector<Real>>("normals"))
{
}

Real
TotalMassFlowRateIntoJunctionAux::computeValue()
{
  Real total_mfr_in = 0;
  for (std::size_t i = 0; i < _normals.size(); i++)
  {
    if (_normals[i] * _rhouA[i] >= 0)
      total_mfr_in += _normals[i] * _rhouA[i];
  }
  return total_mfr_in;
}
