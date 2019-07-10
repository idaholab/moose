#include "THMSpecificVolumeAux.h"

registerMooseObject("THMApp", THMSpecificVolumeAux);

template <>
InputParameters
validParams<THMSpecificVolumeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("rhoA",
                               "Density of the phase (conserved), \alpha \rho A for 2-phase model");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("alpha", 1., "Volume fraction");

  return params;
}

THMSpecificVolumeAux::THMSpecificVolumeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rhoA(coupledValue("rhoA")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

Real
THMSpecificVolumeAux::computeValue()
{
  mooseAssert(_rhoA[_qp] != 0, "Detected zero density.");
  Real v = _alpha[_qp] * _area[_qp] / _rhoA[_qp];
  mooseAssert(v >= 0., "specific volume is negative.");
  return v;
}
