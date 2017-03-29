#include "ReynoldsNumberAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

template <>
InputParameters
validParams<ReynoldsNumberAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addCoupledVar("alpha", 1, "Volume fraction of the phase");
  params.addRequiredCoupledVar("rho", "Density of the phase");
  params.addRequiredCoupledVar("u_vel", "x-component of phase velocity");
  params.addRequiredCoupledVar("Dh", "Hydraulic diameter");
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
    _u_vel(coupledValue("u_vel")),
    _Dh(coupledValue("Dh")),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

ReynoldsNumberAux::~ReynoldsNumberAux() {}

Real
ReynoldsNumberAux::computeValue()
{
  Real visc = _fp.mu(_v[_qp], _e[_qp]);
  return Reynolds(_alpha[_qp], _rho[_qp], _u_vel[_qp], _Dh[_qp], visc);
}
