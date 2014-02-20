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
  params.addRequiredParam<bool>("use_mobility", "If true, then fluxes are multiplied by (density*permeability_nn/viscosity), where the '_nn' indicates the component normal to the boundary.  In this case bare_flux is measured in Pa.s^-1.  This can be used in conjunction with use_relperm.");
  params.addRequiredParam<bool>("use_relperm", "If true, then fluxes are multiplied by relative permeability.  This can be used in conjunction with use_mobility");
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("bare_fluxes", "Tuple of flux values (measured in kg.m^-2.s^-1 for use_mobility=false, and in Pa.s^-1 if use_mobility=true).  A piecewise-linear fit is performed to the (pressure,bare_fluxes) pairs to obtain the flux at any arbitrary pressure.  If a quad-point pressure is less than the first pressure value, the first bare_flux value is used.  If quad-point pressure exceeds the final pressure value, the final bare_flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  return params;
}

RichardsPiecewiseLinearSink::RichardsPiecewiseLinearSink(const std::string & name,
                                             InputParameters parameters) :
    IntegratedBC(name,parameters),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_relperm(getParam<bool>("use_relperm")),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("bare_fluxes"))
{}


Real
RichardsPiecewiseLinearSink::computeQpResidual()
{
  Real flux = _test[_i][_qp]*_sink_func.sample(_u[_qp]);
  return flux;
}

Real
RichardsPiecewiseLinearSink::computeQpJacobian()
{
  Real test_fcn = _test[_i][_qp];
  return test_fcn*_sink_func.sampleDerivative(_u[_qp])*_phi[_j][_qp];
}
