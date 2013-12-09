#include "RichardsPiecewiseLinearSink.h"

#include <iostream>


template<>
InputParameters validParams<RichardsPiecewiseLinearSink>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("fluxes", "Tuple of flux values (measured in kg.m^-2.s^-1).  A piecewise-linear fit is performed to the (pressure,flux) pairs to obtain the flux at any arbitrary pressure.  If a quad-point pressure is less than the first pressure value, the first flux value is used.  If quad-point pressure exceeds the final pressure value, the final flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  return params;
}

RichardsPiecewiseLinearSink::RichardsPiecewiseLinearSink(const std::string & name,
                                             InputParameters parameters) :
    IntegratedBC(name,parameters),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("fluxes")),
    _vel_SUPG(getMaterialProperty<RealVectorValue>("vel_SUPG")),
    _vel_prime_SUPG(getMaterialProperty<RealTensorValue>("vel_prime_SUPG")),
    _tau_SUPG(getMaterialProperty<Real>("tau_SUPG")),
    _tau_prime_SUPG(getMaterialProperty<RealVectorValue>("tau_prime_SUPG"))

{}

/* NOTE: I don't THINK it's necessary to do SUPG here */

Real
RichardsPiecewiseLinearSink::computeQpResidual()
{
  Real test_fcn = _test[_i][_qp] + _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp]*0;
  return test_fcn*_sink_func.sample(_u[_qp]);
}

Real
RichardsPiecewiseLinearSink::computeQpJacobian()
{
  Real test_fcn = _test[_i][_qp] + _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp]*0;
  Real supg_test_prime = (_tau_prime_SUPG[_qp]*_grad_phi[_j][_qp])*(_vel_SUPG[_qp]*_grad_test[_i][_qp]) + _tau_SUPG[_qp]*(_vel_prime_SUPG[_qp]*_grad_phi[_j][_qp])*_grad_test[_i][_qp];
  return test_fcn*_sink_func.sampleDerivative(_u[_qp])*_phi[_j][_qp] + supg_test_prime*_sink_func.sample(_u[_qp])*0;
}
