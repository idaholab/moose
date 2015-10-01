#include "VariableTimeIntegrationAux.h"

template<>
InputParameters validParams<VariableTimeIntegrationAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("variable_to_integrate", "The variable to be integrated");
  params.addParam<Real>("coefficient", 1.0, "A simple coefficient");
  params.addParam<unsigned int>("order",2," the order of global truncation error, midpoint(1), trapazoidal(2),  simpson 1/3 rule (3)");
  return params;
}

VariableTimeIntegrationAux::VariableTimeIntegrationAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _coef(getParam<Real>("coefficient")),
    _order(getParam<unsigned int>("order"))
{
  // Note: initial value should be set by an initial condition.
  Real constant = 1.0/3.0;

  switch (_order)
  {

    case 1:
      _integration_coef.push_back(1.0);
      _coupled_vars.push_back(&coupledValue("variable_to_integrate"));
      break;
    case 2:
      _integration_coef.push_back(0.5);
      _integration_coef.push_back(0.5);
      _coupled_vars.push_back(&coupledValue("variable_to_integrate"));
      _coupled_vars.push_back(&coupledValueOld("variable_to_integrate"));
      break;
    case 3:
      _integration_coef.push_back(constant);
      _integration_coef.push_back(4.0*constant);
      _integration_coef.push_back(constant);
      _coupled_vars.push_back(&coupledValue("variable_to_integrate"));
      _coupled_vars.push_back(&coupledValueOld("variable_to_integrate"));
      _coupled_vars.push_back(&coupledValueOlder("variable_to_integrate"));
      break;
    default:
      _integration_coef.push_back(0.5);
      _integration_coef.push_back(0.5);
      _coupled_vars.push_back(&coupledValue("variable_to_integrate"));
      _coupled_vars.push_back(&coupledValueOld("variable_to_integrate"));
  }
}

Real
VariableTimeIntegrationAux::computeValue()
{
  Real integral = 0.0;
  if(_order == 3)
  {
    if(_dt != _dt_old)
    {
       /*!
        * time step is uneven, so the standard formula will not work. Use a different set of coefficients here.
        */
      Real term1 = -(_dt*_dt - _dt_old*_dt - 2.0*_dt_old*_dt_old)*(*_coupled_vars[2])[_qp]/(6.0*_dt_old);
      Real term2 = (_dt*_dt*_dt + 3.0*_dt*_dt*_dt_old + 3.0*_dt_old*_dt_old*_dt +_dt_old*_dt_old*_dt_old)*(*_coupled_vars[1])[_qp]/(6.0*_dt*_dt_old);
      Real term3 = (2.0*_dt*_dt + _dt*_dt_old - _dt_old*_dt_old)*(*_coupled_vars[0])[_qp]/(6.0*_dt);
      integral = term1 + term2 + term3;

      return _u_older[_qp] + _coef*integral;

     }

     for(unsigned int i=0; i< _order; ++i)
       integral+=_integration_coef[i]*(*_coupled_vars[i])[_qp];

     return _u_older[_qp] + _coef*integral*_dt;
  }

  for(unsigned int i=0; i< _order; ++i)
    integral+=_integration_coef[i]*(*_coupled_vars[i])[_qp];

  return _u_old[_qp] + _coef*integral*_dt;

}

