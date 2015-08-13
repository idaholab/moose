/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SolidMechInertialForceTensors.h"
#include "SubProblem.h"


template<>
InputParameters validParams<SolidMechInertialForceTensors>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Inertial force calculator");
  params.addRequiredCoupledVar("velocity", "velocity variable");
  params.addRequiredCoupledVar("acceleration", "acceleration variable");
  params.addRequiredParam<Real>("beta", "beta parameter");
  params.addRequiredParam<Real>("gamma", "gamma parameter");
  params.addParam<Real>("eta", 0, "eta parameter");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}


SolidMechInertialForceTensors::SolidMechInertialForceTensors(const InputParameters & parameters) :
    Kernel(parameters),
    _density(getMaterialProperty<Real>("density")),
    _u_old(valueOld()),
    _vel_old(coupledValueOld("velocity")),
    _accel_old(coupledValueOld("acceleration")),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _eta(getParam<Real>("eta"))
{}

Real
SolidMechInertialForceTensors::computeQpResidual()
{
  if (_dt == 0)
    return 0;
    Real accel=1/_beta*(((_u[_qp]-_u_old[_qp])/(_dt*_dt)) - _vel_old[_qp]/_dt - _accel_old[_qp]*(0.5-_beta));
    Real vel=_vel_old[_qp] + (_dt*(1-_gamma))*_accel_old[_qp] + _gamma*_dt*accel;
    return _test[_i][_qp]*_density[_qp]*accel+_test[_i][_qp]*_density[_qp]*vel*_eta;
}

Real
SolidMechInertialForceTensors::computeQpJacobian()
{
  if (_dt == 0)
    return 0;
  return _test[_i][_qp]*_density[_qp]/(_beta*_dt*_dt)*_phi[_j][_qp]+_eta*_test[_i][_qp]*_density[_qp]*_gamma/_beta/_dt*_phi[_j][_qp];
}
