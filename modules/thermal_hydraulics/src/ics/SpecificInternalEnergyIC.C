#include "SpecificInternalEnergyIC.h"

registerMooseObject("ThermalHydraulicsApp", SpecificInternalEnergyIC);

InputParameters
SpecificInternalEnergyIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  return params;
}

SpecificInternalEnergyIC::SpecificInternalEnergyIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _rho(coupledValue("rhoA")),
    _rhou(coupledValue("rhouA")),
    _rhoE(coupledValue("rhoEA"))
{
}

Real
SpecificInternalEnergyIC::value(const Point & /*p*/)
{
  return (_rhoE[_qp] - 0.5 * _rhou[_qp] * _rhou[_qp] / _rho[_qp]) / _rho[_qp];
}
