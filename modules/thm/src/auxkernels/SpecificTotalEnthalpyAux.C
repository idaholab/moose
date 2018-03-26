#include "SpecificTotalEnthalpyAux.h"

registerMooseObject("RELAP7App", SpecificTotalEnthalpyAux);

template <>
InputParameters
validParams<SpecificTotalEnthalpyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("alpha", 1., "Volume fraction");

  return params;
}

SpecificTotalEnthalpyAux::SpecificTotalEnthalpyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rhoA(coupledValue("rhoA")),
    _rhoEA(coupledValue("rhoEA")),
    _pressure(coupledValue("p")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

Real
SpecificTotalEnthalpyAux::computeValue()
{
  return (_rhoEA[_qp] + _alpha[_qp] * _pressure[_qp] * _area[_qp]) / _rhoA[_qp];
}
