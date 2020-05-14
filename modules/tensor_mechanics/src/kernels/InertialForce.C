//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InertialForce.h"
#include "SubProblem.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"

registerMooseObject("TensorMechanicsApp", InertialForce);

InputParameters
InertialForce::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addClassDescription("Calculates the residual for the inertial force "
                             "($M \\cdot acceleration$) and the contribution of mass"
                             " dependent Rayleigh damping and HHT time "
                             " integration scheme ($\\eta \\cdot M \\cdot"
                             " ((1+\\alpha)velq2-\\alpha \\cdot vel-old) $)");
  params.set<bool>("use_displaced_mesh") = true;
  params.addCoupledVar("velocity", "velocity variable");
  params.addCoupledVar("acceleration", "acceleration variable");
  params.addParam<Real>("beta", "beta parameter for Newmark Time integration");
  params.addParam<Real>("gamma", "gamma parameter for Newmark Time integration");
  params.addParam<MaterialPropertyName>("eta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the eta parameter for the "
                                        "Rayleigh damping.");
  params.addParam<Real>("alpha",
                        0,
                        "alpha parameter for mass dependent numerical damping induced "
                        "by HHT time integration scheme");
  params.addParam<MaterialPropertyName>(
      "density", "density", "Name of Material Property that provides the density");
  return params;
}

InertialForce::InertialForce(const InputParameters & parameters)
  : TimeKernel(parameters),
    _density(getMaterialProperty<Real>("density")),
    _has_beta(isParamValid("beta")),
    _has_gamma(isParamValid("gamma")),
    _beta(_has_beta ? getParam<Real>("beta") : 0.1),
    _gamma(_has_gamma ? getParam<Real>("gamma") : 0.1),
    _has_velocity(isParamValid("velocity")),
    _has_acceleration(isParamValid("acceleration")),
    _eta(getMaterialProperty<Real>("eta")),
    _alpha(getParam<Real>("alpha")),
    _time_integrator(*_sys.getTimeIntegrator())
{
  if (_has_beta && _has_gamma && _has_velocity && _has_acceleration)
  {
    _vel_old = &coupledValueOld("velocity");
    _accel_old = &coupledValueOld("acceleration");
    _u_old = &valueOld();
  }
  else if (!_has_beta && !_has_gamma && !_has_velocity && !_has_acceleration)
  {
    _u_dot_old = &(_var.uDotOld());
    _du_dot_du = &(_var.duDotDu());
    _du_dotdot_du = &(_var.duDotDotDu());

    addFEVariableCoupleableVectorTag(_time_integrator.uDotFactorTag());
    addFEVariableCoupleableVectorTag(_time_integrator.uDotDotFactorTag());

    _u_dot_factor = &_var.vectorTagValue(_time_integrator.uDotFactorTag());
    _u_dotdot_factor = &_var.vectorTagValue(_time_integrator.uDotDotFactorTag());

    if (_time_integrator.isLumped())
    {
      _u_dot_factor_dof = &_var.vectorTagDofValue(_time_integrator.uDotFactorTag());
      _u_dotdot_factor_dof = &_var.vectorTagDofValue(_time_integrator.uDotDotFactorTag());
    }
  }
  else
    mooseError("InertialForce: Either all or none of `beta`, `gamma`, `velocity`and `acceleration` "
               "should be provided as input.");

  // Check if HHT and explicit are being used simultaneously
  if (_alpha != 0 && _time_integrator.isExplicit())
    mooseError("InertialForce: HHT time integration parameter can only be used with Newmark-Beta "
               "time integration.");

  // Check if beta and explicit are being used simultaneously
  if (_has_beta && _time_integrator.isExplicit())
    mooseError("InertialForce: Newmark-beta integration parameter, beta, cannot be provided along "
               "with an explicit time "
               "integrator.");
}

Real
InertialForce::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else if (_has_beta)
  {
    Real accel = 1. / _beta *
                 (((_u[_qp] - (*_u_old)[_qp]) / (_dt * _dt)) - (*_vel_old)[_qp] / _dt -
                  (*_accel_old)[_qp] * (0.5 - _beta));
    Real vel = (*_vel_old)[_qp] + (_dt * (1 - _gamma)) * (*_accel_old)[_qp] + _gamma * _dt * accel;
    return _test[_i][_qp] * _density[_qp] *
           (accel + vel * _eta[_qp] * (1 + _alpha) - _alpha * _eta[_qp] * (*_vel_old)[_qp]);
  }

  // Lumped mass option
  // Only lumping the masses here
  // will multiply by corresponding residual multiplier after lumping the matrix
  else if (_time_integrator.isLumped() && _time_integrator.isExplicit())
    return _test[_i][_qp] * _density[_qp];

  // Consistent mass option
  // Same for explicit, implicit, and implicit with HHT
  else
    return _test[_i][_qp] * _density[_qp] *
           ((*_u_dotdot_factor)[_qp] + (*_u_dot_factor)[_qp] * _eta[_qp] * (1.0 + _alpha) -
            _alpha * _eta[_qp] * (*_u_dot_old)[_qp]);
}

void
InertialForce::computeResidualAdditional()
{
  if (_dt == 0)
    return;

  // Explicit lumped only
  if (!_time_integrator.isLumped() || !_time_integrator.isExplicit())
    return;

  for (unsigned int i = 0; i < _test.size(); ++i)
    _local_re(i) *= (*_u_dotdot_factor_dof)[i] + _eta[0] * (*_u_dot_factor_dof)[i];
}

Real
InertialForce::computeQpJacobian()
{
  if (_dt == 0)
    return 0;
  else
  {
    if (_has_beta)
      return _test[_i][_qp] * _density[_qp] / (_beta * _dt * _dt) * _phi[_j][_qp] +
             _eta[_qp] * (1 + _alpha) * _test[_i][_qp] * _density[_qp] * _gamma / _beta / _dt *
                 _phi[_j][_qp];
    else
      return _test[_i][_qp] * _density[_qp] * (*_du_dotdot_du)[_qp] * _phi[_j][_qp] +
             _eta[_qp] * (1 + _alpha) * _test[_i][_qp] * _density[_qp] * (*_du_dot_du)[_qp] *
                 _phi[_j][_qp];
  }
}
