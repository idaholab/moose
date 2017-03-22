/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the mass due to a half-gaussian sink flux from the boundary of a
//  volume.
//
#include "RichardsHalfGaussianSinkFlux.h"
#include "Function.h"

template <>
InputParameters
validParams<RichardsHalfGaussianSinkFlux>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<Real>("max",
                                "Maximum of the flux (measured in kg.m^-2.s^-1).  Flux out "
                                "= max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and Flux "
                                "out = max for p>centre.  Note, to make this a source "
                                "rather than a sink, let max<0");
  params.addRequiredParam<Real>("sd",
                                "Standard deviation of the Gaussian (measured in Pa).  Flux "
                                "out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and "
                                "Flux out = max for p>centre.");
  params.addRequiredParam<Real>("centre",
                                "Centre of the Gaussian (measured in Pa).  Flux out = "
                                "max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and "
                                "Flux out = max for p>centre.");
  params.addParam<FunctionName>(
      "multiplying_fcn",
      1.0,
      "The flux will be multiplied by this spatially-and-temporally varying function.");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  return params;
}

RichardsHalfGaussianSinkFlux::RichardsHalfGaussianSinkFlux(const InputParameters & parameters)
  : SideIntegralVariablePostprocessor(parameters),
    _feproblem(dynamic_cast<FEProblemBase &>(_subproblem)),
    _maximum(getParam<Real>("max")),
    _sd(getParam<Real>("sd")),
    _centre(getParam<Real>("centre")),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(coupled("variable"))),
    _m_func(getFunction("multiplying_fcn")),
    _pp(getMaterialProperty<std::vector<Real>>("porepressure"))
{
}

Real
RichardsHalfGaussianSinkFlux::computeQpIntegral()
{
  if (_pp[_qp][_pvar] >= _centre)
    return _maximum * _dt * _m_func.value(_t, _q_point[_qp]);
  else
    return _maximum * exp(-0.5 * std::pow((_pp[_qp][_pvar] - _centre) / _sd, 2)) * _dt *
           _m_func.value(_t, _q_point[_qp]);
}
