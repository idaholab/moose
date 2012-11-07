#include "TemperatureAux.h"
#include "EquationOfState.h"

template<>
InputParameters validParams<TemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Coupled variables.  We assume that temperature is always a function of rho, rhou, and rhoE...
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "momentum");
  params.addRequiredCoupledVar("rhoE", "total energy");

  // The EOS function is a required parameter.
  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
  
  return params;
}


TemperatureAux::TemperatureAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),
    _eos(getUserObject<EquationOfState>("eos"))
{}


TemperatureAux::~TemperatureAux()
{
  // Destructor, empty
}

Real TemperatureAux::computeValue()
{
  return _eos.temperature(_rho[_qp], _rhou[_qp], _rhoE[_qp]);
}
