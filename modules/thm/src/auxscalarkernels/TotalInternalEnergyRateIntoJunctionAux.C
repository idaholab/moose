#include "TotalInternalEnergyRateIntoJunctionAux.h"

registerMooseObject("THMApp", TotalInternalEnergyRateIntoJunctionAux);

template <>
InputParameters
validParams<TotalInternalEnergyRateIntoJunctionAux>()
{
  InputParameters params = validParams<AuxNodalScalarKernel>();
  params.addRequiredCoupledVar("rhoA", "Density");
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addRequiredCoupledVar("rhoEA", "Total energy");
  params.addRequiredParam<std::vector<Real>>("normals",
                                             "Component of outward normals along 1-D direction");
  return params;
}

TotalInternalEnergyRateIntoJunctionAux::TotalInternalEnergyRateIntoJunctionAux(
    const InputParameters & parameters)
  : AuxNodalScalarKernel(parameters),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),
    _normals(getParam<std::vector<Real>>("normals"))
{
}

Real
TotalInternalEnergyRateIntoJunctionAux::computeValue()
{
  Real total_internal_energy_in = 0;
  for (std::size_t i = 0; i < _rhouA.size(); i++)
  {
    if (_normals[i] * _rhouA[i] > 0)
      total_internal_energy_in +=
          _normals[i] * _rhouA[i] / _rhoA[i] * (_rhoEA[i] - 0.5 * _rhouA[i] * _rhouA[i] / _rhoA[i]);
  }
  return total_internal_energy_in;
}
