#include "SpecificTotalEnthalpyIC.h"

registerMooseObject("THMApp", SpecificTotalEnthalpyIC);

template <>
InputParameters
validParams<SpecificTotalEnthalpyIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("alpha", 1., "Volume fraction");

  return params;
}

SpecificTotalEnthalpyIC::SpecificTotalEnthalpyIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _rhoA(coupledValue("rhoA")),
    _rhoEA(coupledValue("rhoEA")),
    _pressure(coupledValue("p")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

Real
SpecificTotalEnthalpyIC::value(const Point & /*p*/)
{
  return (_rhoEA[_qp] + _alpha[_qp] * _pressure[_qp] * _area[_qp]) / _rhoA[_qp];
}
