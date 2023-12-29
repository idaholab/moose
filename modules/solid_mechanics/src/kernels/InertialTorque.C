//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InertialTorque.h"

#include "MooseVariable.h"
#include "PermutationTensor.h"

registerMooseObject("TensorMechanicsApp", InertialTorque);

InputParameters
InertialTorque::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addClassDescription("Kernel for inertial torque: density * displacement x acceleration");
  params.set<bool>("use_displaced_mesh") = false;
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component<3",
      "The component of the Cosserat rotation Variable prescribed to "
      "this Kernel (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("displacements", "The 3 displacement variables");
  params.addRequiredCoupledVar("velocities", "The 3 velocity variables");
  params.addRequiredCoupledVar("accelerations", "The 3 acceleration variables");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark Time integration");
  params.addRequiredParam<Real>("gamma", "gamma parameter for Newmark Time integration");
  params.addParam<MaterialPropertyName>(
      "density", "density", "Name of Material Property that provides the density");
  params.addParam<MaterialPropertyName>("eta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the eta parameter for the "
                                        "Rayleigh damping.");
  params.addParam<Real>("alpha",
                        0,
                        "alpha parameter for mass dependent numerical damping induced "
                        "by HHT time integration scheme");
  return params;
}

InertialTorque::InertialTorque(const InputParameters & parameters)
  : TimeKernel(parameters),
    _density(getMaterialProperty<Real>("density")),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _eta(getMaterialProperty<Real>("eta")),
    _alpha(getParam<Real>("alpha")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_num(coupledIndices("displacements")),
    _disp(coupledValues("displacements")),
    _disp_old(coupledValuesOld("displacements")),
    _vel_old(coupledValuesOld("velocities")),
    _accel_old(coupledValuesOld("accelerations")),
    _accel(_ndisp),
    _vel(_ndisp),
    _daccel(_ndisp),
    _dvel(_ndisp)
{
  if (_ndisp != 3 || coupledComponents("velocities") != 3 ||
      coupledComponents("accelerations") != 3)
    mooseError(
        "InertialTorque: This Kernel is only defined for 3-dimensional simulations so 3 "
        "displacement variables, 3 velocity variables and 3 acceleration variables are needed");
}

Real
InertialTorque::computeQpResidual()
{
  for (unsigned int j = 0; j < _ndisp; ++j)
  {
    // Newmark and damping
    _accel[j] = 1.0 / _beta *
                ((((*_disp[j])[_qp] - (*_disp_old[j])[_qp]) / (_dt * _dt)) -
                 (*_vel_old[j])[_qp] / _dt - (*_accel_old[j])[_qp] * (0.5 - _beta));
    _vel[j] = (*_vel_old[j])[_qp] + (_dt * (1 - _gamma)) * (*_accel_old[j])[_qp] +
              _gamma * _dt * _accel[j];
    _accel[j] =
        _accel[j] + _vel[j] * _eta[_qp] * (1 + _alpha) - _alpha * _eta[_qp] * (*_vel_old[j])[_qp];
  }

  Real the_sum = 0.0;
  for (unsigned int j = 0; j < _ndisp; ++j)
    for (unsigned int k = 0; k < _ndisp; ++k)
      the_sum += PermutationTensor::eps(_component, j, k) * (*_disp[j])[_qp] * _accel[k];
  return _test[_i][_qp] * _density[_qp] * the_sum;
}

Real
InertialTorque::computeQpJacobian()
{
  return InertialTorque::computeQpOffDiagJacobian(_var.number());
}

Real
InertialTorque::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int j = 0; j < _ndisp; ++j)
  {
    _accel[j] = 1.0 / _beta *
                ((((*_disp[j])[_qp] - (*_disp_old[j])[_qp]) / (_dt * _dt)) -
                 (*_vel_old[j])[_qp] / _dt - (*_accel_old[j])[_qp] * (0.5 - _beta));
    _daccel[j] = 1.0 / _beta / _dt / _dt;
    _vel[j] = (*_vel_old[j])[_qp] + (_dt * (1 - _gamma)) * (*_accel_old[j])[_qp] +
              _gamma * _dt * _accel[j];
    _dvel[j] = _gamma * _dt * _daccel[j];
    _accel[j] =
        _accel[j] + _vel[j] * _eta[_qp] * (1 + _alpha) - _alpha * _eta[_qp] * (*_vel_old[j])[_qp];
    _daccel[j] = _daccel[j] + _dvel[j] * _eta[_qp] * (1 + _alpha);
  }

  for (unsigned v = 0; v < _ndisp; ++v)
  {
    if (v == _component)
      continue; // PermutationTensor will kill this
    if (jvar == _disp_num[v])
    {
      Real the_sum = 0.0;
      for (unsigned int k = 0; k < _ndisp; ++k)
        the_sum += PermutationTensor::eps(_component, v, k) * _accel[k];
      for (unsigned int j = 0; j < _ndisp; ++j)
        the_sum += PermutationTensor::eps(_component, j, v) * (*_disp[j])[_qp] * _daccel[v];
      return _test[_i][_qp] * _density[_qp] * the_sum * _phi[_j][_qp];
    }
  }
  return 0.0;
}
