#include "ReynoldsNumberAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("RELAP7App", ReynoldsNumberAux);

template <>
InputParameters
validParams<ReynoldsNumberAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addCoupledVar("alpha", 1, "Volume fraction of the phase");
  params.addRequiredCoupledVar("rho", "Density of the phase");
  params.addRequiredCoupledVar("vel", "x-component of phase velocity");
  params.addRequiredCoupledVar("D_h", "Hydraulic diameter");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the user object with fluid properties");
  return params;
}

ReynoldsNumberAux::ReynoldsNumberAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _alpha(coupledValue("alpha")),
    _rho(coupledValue("rho")),
    _vel(coupledValue("vel")),
    _D_h(coupledValue("D_h")),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
ReynoldsNumberAux::computeValue()
{
  Real visc = _fp.mu_from_v_e(_v[_qp], _e[_qp]);
  return THM::Reynolds(_alpha[_qp], _rho[_qp], _vel[_qp], _D_h[_qp], visc);
}
