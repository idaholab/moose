#include "SpecificVolumeAux.h"

registerMooseObject("THMApp", SpecificVolumeAux);

template <>
InputParameters
validParams<SpecificVolumeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("rhoA",
                               "Density of the phase (conserved), \alpha \rho A for 2-phase model");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("alpha", 1., "Volume fraction");

  return params;
}

SpecificVolumeAux::SpecificVolumeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rhoA(coupledValue("rhoA")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

Real
SpecificVolumeAux::computeValue()
{
  mooseAssert(_rhoA[_qp] != 0, "Detected zero density.");
  return _alpha[_qp] * _area[_qp] / _rhoA[_qp];
}
