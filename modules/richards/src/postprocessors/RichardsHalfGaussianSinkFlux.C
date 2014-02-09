/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  This post processor returns the mass due to a half-gaussian sink flux from the boundary of a volume.
//
#include "RichardsHalfGaussianSinkFlux.h"

template<>
InputParameters validParams<RichardsHalfGaussianSinkFlux>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<Real>("max", "Maximum of the flux (measured in kg.m^-2.s^-1).  Flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and Flux out = max for p>centre.  Note, to make this a source rather than a sink, let max<0");
  params.addRequiredParam<Real>("sd", "Standard deviation of the Gaussian (measured in Pa).  Flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and Flux out = max for p>centre.");
  params.addRequiredParam<Real>("centre", "Centre of the Gaussian (measured in Pa).  Flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and Flux out = max for p>centre.");
  return params;
}

RichardsHalfGaussianSinkFlux::RichardsHalfGaussianSinkFlux(const std::string & name, InputParameters parameters) :
    SideIntegralVariablePostprocessor(name, parameters),
    _feproblem(dynamic_cast<FEProblem &>(_subproblem)),
    _maximum(getParam<Real>("max")),
    _sd(getParam<Real>("sd")),
    _centre(getParam<Real>("centre"))
{}

Real
RichardsHalfGaussianSinkFlux::computeQpIntegral()
{
  if (_u[_qp] >= _centre) {
    return _maximum*_feproblem.dt();
  }
  else {
    return _maximum*exp(-0.5*std::pow((_u[_qp] - _centre)/_sd, 2))*_feproblem.dt();
  }
}
