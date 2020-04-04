//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsHalfGaussianSink.h"

// MOOSEincludes
#include "Function.h"
#include "Material.h"
#include "MooseVariable.h"

#include "libmesh/utility.h"

registerMooseObject("RichardsApp", RichardsHalfGaussianSink);

InputParameters
RichardsHalfGaussianSink::validParams()
{
  InputParameters params = IntegratedBC::validParams();
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
  params.addParam<FunctionName>("multiplying_fcn",
                                1.0,
                                "If this function is provided, the flux "
                                "will be multiplied by this function.  "
                                "This is useful for spatially or "
                                "temporally varying sinks");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  return params;
}

RichardsHalfGaussianSink::RichardsHalfGaussianSink(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _maximum(getParam<Real>("max")),
    _sd(getParam<Real>("sd")),
    _centre(getParam<Real>("centre")),
    _m_func(getFunction("multiplying_fcn")),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),
    _pp(getMaterialProperty<std::vector<Real>>("porepressure")),
    _dpp_dv(getMaterialProperty<std::vector<std::vector<Real>>>("dporepressure_dv"))
{
}

Real
RichardsHalfGaussianSink::computeQpResidual()
{
  const Real test_fcn_f = _test[_i][_qp] * _m_func.value(_t, _q_point[_qp]);

  if (_pp[_qp][_pvar] >= _centre)
    return test_fcn_f * _maximum;

  return test_fcn_f * _maximum *
         std::exp(-0.5 * Utility::pow<2>((_pp[_qp][_pvar] - _centre) / _sd));
}

Real
RichardsHalfGaussianSink::computeQpJacobian()
{
  if (_pp[_qp][_pvar] >= _centre)
    return 0.0;

  const Real test_fcn_f = _test[_i][_qp] * _m_func.value(_t, _q_point[_qp]);
  return -test_fcn_f * _maximum * (_pp[_qp][_pvar] - _centre) / Utility::pow<2>(_sd) *
         std::exp(-0.5 * Utility::pow<2>((_pp[_qp][_pvar] - _centre) / _sd)) * _phi[_j][_qp] *
         _dpp_dv[_qp][_pvar][_pvar];
}

Real
RichardsHalfGaussianSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;

  if (_pp[_qp][_pvar] >= _centre)
    return 0.0;

  const Real test_fcn_f = _test[_i][_qp] * _m_func.value(_t, _q_point[_qp]);
  const unsigned int dvar = _richards_name_UO.richards_var_num(jvar);
  return -test_fcn_f * _maximum * (_pp[_qp][_pvar] - _centre) / Utility::pow<2>(_sd) *
         std::exp(-0.5 * Utility::pow<2>((_pp[_qp][_pvar] - _centre) / _sd)) * _phi[_j][_qp] *
         _dpp_dv[_qp][_pvar][dvar];
}
