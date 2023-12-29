//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicStressDivergenceTensors.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", DynamicStressDivergenceTensors);

InputParameters
DynamicStressDivergenceTensors::validParams()
{
  InputParameters params = StressDivergenceTensors::validParams();
  params.addClassDescription(
      "Residual due to stress related Rayleigh damping and HHT time integration terms ");
  params.addParam<MaterialPropertyName>("zeta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the zeta parameter for the "
                                        "Rayleigh damping.");
  params.addParam<Real>("alpha", 0, "alpha parameter for HHT time integration");
  params.addParam<bool>("static_initialization",
                        false,
                        "Set to true to get the system to "
                        "equilibrium under gravity by running a "
                        "quasi-static analysis (by solving Ku = F) "
                        "in the first time step");
  return params;
}

DynamicStressDivergenceTensors::DynamicStressDivergenceTensors(const InputParameters & parameters)
  : StressDivergenceTensors(parameters),
    _stress_older(getMaterialPropertyOlderByName<RankTwoTensor>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress")),
    _zeta(getMaterialProperty<Real>("zeta")),
    _alpha(getParam<Real>("alpha")),
    _static_initialization(getParam<bool>("static_initialization"))
{
}

Real
DynamicStressDivergenceTensors::computeQpResidual()
{
  /**
   *This kernel needs to be used only if either Rayleigh damping or numerical damping through HHT
   *time integration scheme needs to be added to the problem through the stiffness dependent damping
   * parameter _zeta or HHT parameter _alpha, respectively.
   *
   * The residual of _zeta*K*[(1+_alpha)vel-_alpha vel_old]+ alpha K [ u - uold] + K u is required
   * = _zeta*[(1+_alpha)d/dt (Div sigma)-alpha d/dt(Div sigma_old)] +alpha [Div sigma - Div
   *sigma_old]+ Div sigma
   * = _zeta*[(1+alpha)(Div sigma - Div sigma_old)/dt - alpha (Div sigma_old - Div sigma_older)/dt]
   *   + alpha [Div sigma - Div sigma_old] +Div sigma
   * = [(1+_alpha)*_zeta/dt +_alpha+1]* Div sigma - [(1+2_alpha)*_zeta/dt + _alpha] Div sigma_old +
   *_alpha*_zeta/dt Div sigma_older
   */

  Real residual = 0.0;
  if (_static_initialization && _t == _dt)
  {
    // If static initialization is true, then in the first step residual is only Ku which is
    // stress.grad(test).
    residual += _stress[_qp].row(_component) * _grad_test[_i][_qp];

    if (_volumetric_locking_correction)
      residual += _stress[_qp].trace() / 3.0 *
                  (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component));
  }
  else if (_dt > 0)
  {
    residual +=
        _stress[_qp].row(_component) * _grad_test[_i][_qp] *
            (1.0 + _alpha + (1.0 + _alpha) * _zeta[_qp] / _dt) -
        (_alpha + (1.0 + 2.0 * _alpha) * _zeta[_qp] / _dt) * _stress_old[_qp].row(_component) *
            _grad_test[_i][_qp] +
        (_alpha * _zeta[_qp] / _dt) * _stress_older[_qp].row(_component) * _grad_test[_i][_qp];

    if (_volumetric_locking_correction)
      residual += (_stress[_qp].trace() * (1.0 + _alpha + (1.0 + _alpha) * _zeta[_qp] / _dt) -
                   (_alpha + (1.0 + 2.0 * _alpha) * _zeta[_qp] / _dt) * _stress_old[_qp].trace() +
                   (_alpha * _zeta[_qp] / _dt) * _stress_older[_qp].trace()) /
                  3.0 * (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component));
  }

  return residual;
}

Real
DynamicStressDivergenceTensors::computeQpJacobian()
{
  if (_static_initialization && _t == _dt)
    return StressDivergenceTensors::computeQpJacobian();
  else if (_dt > 0)
    return StressDivergenceTensors::computeQpJacobian() *
           (1.0 + _alpha + (1.0 + _alpha) * _zeta[_qp] / _dt);
  else
    return 0.0;
}

Real
DynamicStressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  bool active = true;

  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var[i])
      active = true;

  if (active)
  {
    if (_static_initialization && _t == _dt)
      return StressDivergenceTensors::computeQpOffDiagJacobian(jvar);
    else if (_dt > 0)
      return StressDivergenceTensors::computeQpOffDiagJacobian(jvar) *
             (1.0 + _alpha + (1.0 + _alpha) * _zeta[_qp] / _dt);
    else
      return 0.0;
  }

  return 0;
}
