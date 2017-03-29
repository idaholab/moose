#include "PrandtlNumberAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

template <>
InputParameters
validParams<PrandtlNumberAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the user object with fluid properties");
  return params;
}

PrandtlNumberAux::PrandtlNumberAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

PrandtlNumberAux::~PrandtlNumberAux() {}

Real
PrandtlNumberAux::computeValue()
{
  Real cp = _fp.cp(_v[_qp], _e[_qp]);
  Real mu = _fp.mu(_v[_qp], _e[_qp]);
  Real k = _fp.k(_v[_qp], _e[_qp]);
  return Prandtl(cp, mu, k);
}
