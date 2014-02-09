/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

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
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("fluxes"))
{}


Real
RichardsPiecewiseLinearSink::computeQpResidual()
{
  Real test_fcn = _test[_i][_qp];
  return test_fcn*_sink_func.sample(_u[_qp]);
}

Real
RichardsPiecewiseLinearSink::computeQpJacobian()
{
  Real test_fcn = _test[_i][_qp];
  return test_fcn*_sink_func.sampleDerivative(_u[_qp])*_phi[_j][_qp];
}
