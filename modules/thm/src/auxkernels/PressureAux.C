#include "PressureAux.h"
#include "EquationOfState.h"

template<>
InputParameters validParams<PressureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Required coupled variables.  We assume that pressure always at least depends on density...
  params.addRequiredCoupledVar("rho", "density");

  // Optional coupled variables.  Pressure may or may not also depend on momentum and total energy.
  params.addCoupledVar("rhou", "momentum");
  params.addCoupledVar("rhoE", "total energy");
  
  // The EOS function is a required parameter.
  params.addRequiredParam<std::string>("eos_function", "The EOS function object");

  return params;
}

PressureAux::PressureAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _rho(coupledValue("rho")),
    _rhou(isCoupled("rhou") ? coupledValue("rhou") : _zero),
    _rhoE(isCoupled("rhoE") ? coupledValue("rhoE") : _zero),
    _func(getFunction("eos_function")),
    _eos( dynamic_cast<EquationOfState&>(_func))
{}

Real
PressureAux::computeValue()
{
  return _eos.pressure(_rho[_qp], _rhou[_qp], _rhoE[_qp]);
}
