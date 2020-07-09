//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSAD3Eqn.h"
#include "INSADObjectTracker.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSAD3Eqn);

InputParameters
INSAD3Eqn::validParams()
{
  InputParameters params = INSADMaterial::validParams();
  params.addClassDescription("This material computes properties needed for stabilized formulations "
                             "of the mass, momentum, and energy equations.");
  params.addRequiredCoupledVar("temperature", "The temperature");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  return params;
}

INSAD3Eqn::INSAD3Eqn(const InputParameters & parameters)
  : INSADMaterial(parameters),
    _temperature(adCoupledValue("temperature")),
    _grad_temperature(adCoupledGradient("temperature")),
    _temperature_dot(nullptr),
    _cp(getADMaterialProperty<Real>("cp_name")),
    _temperature_advective_strong_residual(
        declareADProperty<Real>("temperature_advective_strong_residual")),
    _temperature_td_strong_residual(declareADProperty<Real>("temperature_td_strong_residual")),
    _temperature_ambient_convection_strong_residual(
        declareADProperty<Real>("temperature_ambient_convection_strong_residual")),
    _temperature_source_strong_residual(
        declareADProperty<Real>("temperature_source_strong_residual")),
    _heat_source_var(nullptr),
    _heat_source_function(nullptr)
{
}

void
INSAD3Eqn::initialSetup()
{
  INSADMaterial::initialSetup();

  if (_has_transient)
    _temperature_dot = &adCoupledDot("temperature");

  if ((_has_ambient_convection = _object_tracker->get<bool>("has_ambient_convection")))
  {
    _ambient_convection_alpha = _object_tracker->get<Real>("ambient_convection_alpha");
    _ambient_temperature = _object_tracker->get<Real>("ambient_temperature");
  }

  if ((_has_heat_source = _object_tracker->get<bool>("has_heat_source")))
  {
    if (_object_tracker->isTrackerParamValid("heat_source_var"))
      _heat_source_var = _object_tracker->get<const ADVariableValue *>("heat_source_var");
    else if (_object_tracker->isTrackerParamValid("heat_source_function"))
      _heat_source_function = _object_tracker->get<const Function *>("heat_source_function");
  }
}

void
INSAD3Eqn::computeQpProperties()
{
  INSADMaterial::computeQpProperties();

  // For the remaining terms we make individual properties so they can be consumed by non-SUPG
  // kernels. This avoids double calculation for the non-supg and supg parts of the residual. We
  // don't need an individual property for the conductive term because the corresponding non-supg
  // contribution is integrated by parts and hence there is no double calculation (the 'weak' and
  // 'strong' terms are diferent in this case)

  _temperature_advective_strong_residual[_qp] =
      _rho[_qp] * _cp[_qp] * _velocity[_qp] * _grad_temperature[_qp];

  if (_has_transient)
  {
    mooseAssert(_temperature_dot, "The temperature time derivative is null");
    _temperature_td_strong_residual[_qp] = _cp[_qp] * _rho[_qp] * (*_temperature_dot)[_qp];
  }

  if (_has_ambient_convection)
    _temperature_ambient_convection_strong_residual[_qp] =
        _ambient_convection_alpha * (_temperature[_qp] - _ambient_temperature);

  if (_has_heat_source)
  {
    if (_heat_source_var)
      _temperature_source_strong_residual[_qp] = -(*_heat_source_var)[_qp];
    else
    {
      mooseAssert(_heat_source_function,
                  "Either the heat source var or the heat source function must be non-null in "
                  "'INSAD3Eqn'");
      _temperature_source_strong_residual[_qp] = -_heat_source_function->value(_t, _q_point[_qp]);
    }
  }
}
