#include "THMSpecificInternalEnergyAux.h"

registerMooseObject("THMApp", THMSpecificInternalEnergyAux);

InputParameters
THMSpecificInternalEnergyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  return params;
}

THMSpecificInternalEnergyAux::THMSpecificInternalEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue("rhoA")),
    _rhou(coupledValue("rhouA")),
    _rhoE(coupledValue("rhoEA"))
{
}

Real
THMSpecificInternalEnergyAux::computeValue()
{
  return (_rhoE[_qp] - 0.5 * _rhou[_qp] * _rhou[_qp] / _rho[_qp]) / _rho[_qp];
}
