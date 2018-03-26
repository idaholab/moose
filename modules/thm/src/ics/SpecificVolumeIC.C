#include "SpecificVolumeIC.h"

registerMooseObject("RELAP7App", SpecificVolumeIC);

template <>
InputParameters
validParams<SpecificVolumeIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("rhoA",
                               "Density of the phase (conserved), \alpha \rho A for 2-phase model");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  return params;
}

SpecificVolumeIC::SpecificVolumeIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _rhoA(coupledValue("rhoA")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

Real
SpecificVolumeIC::value(const Point & /*p*/)
{
  mooseAssert(_rhoA[_qp] != 0, "Detected zero density.");
  return _alpha[_qp] * _area[_qp] / _rhoA[_qp];
}
