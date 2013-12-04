//  This post processor returns the mass due to a flux from the boundary of a volume.
//
#include "RichardsPiecewiseLinearSinkFlux.h"

template<>
InputParameters validParams<RichardsPiecewiseLinearSinkFlux>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("fluxes", "Tuple of flux values (measured in kg.m^-2.s^-1).  A piecewise-linear fit is performed to the (pressure,flux) pairs to obtain the flux at any arbitrary pressure.  If a quad-point pressure is less than the first pressure value, the first flux value is used.  If quad-point pressure exceeds the final pressure value, the final flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  return params;
}

RichardsPiecewiseLinearSinkFlux::RichardsPiecewiseLinearSinkFlux(const std::string & name, InputParameters parameters) :
    SideIntegralVariablePostprocessor(name, parameters),
    _feproblem(dynamic_cast<FEProblem &>(_subproblem)),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("fluxes"))
{}

Real
RichardsPiecewiseLinearSinkFlux::computeQpIntegral()
{
  return _sink_func.sample(_u[_qp])*_feproblem.dt();
}
