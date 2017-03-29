#include "MachNumberAux.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

template <>
InputParameters
validParams<MachNumberAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("u_vel", "x-component of phase velocity");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");
  return params;
}

MachNumberAux::MachNumberAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _u_vel(coupledValue("u_vel")),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

MachNumberAux::~MachNumberAux() {}

Real
MachNumberAux::computeValue()
{
  Real speed_of_sound = _fp.c(_v[_qp], _e[_qp]);
  return _u_vel[_qp] / speed_of_sound;
}
