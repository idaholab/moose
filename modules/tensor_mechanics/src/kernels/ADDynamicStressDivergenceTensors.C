//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDynamicStressDivergenceTensors.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", ADDynamicStressDivergenceTensors);

InputParameters
ADDynamicStressDivergenceTensors::validParams()
{
  InputParameters params = ADStressDivergenceTensors::validParams();
  params.addClassDescription(
      "Residual due to stress related Rayleigh damping and HHT time integration terms");
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

ADDynamicStressDivergenceTensors::ADDynamicStressDivergenceTensors(
    const InputParameters & parameters)
  : ADStressDivergenceTensors(parameters),
    _stress_older(getMaterialPropertyOlder<RankTwoTensor>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _zeta(getMaterialProperty<Real>("zeta")),
    _alpha(getParam<Real>("alpha")),
    _static_initialization(getParam<bool>("static_initialization"))
{
}

ADReal
ADDynamicStressDivergenceTensors::computeQpResidual()
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

  ADReal residual;
  if (_static_initialization && _t == _dt)
  {
    // If static initialization is true, then in the first step residual is only Ku which is
    // stress.grad(test).
    residual = _stress[_qp].row(_component) * _grad_test[_i][_qp];

    if (_volumetric_locking_correction)
      residual +=
          _stress[_qp].trace() / 3.0 * (_avg_grad_test[_i] - _grad_test[_i][_qp](_component));
  }
  else if (_dt > 0)
  {
    residual =
        _stress[_qp].row(_component) * _grad_test[_i][_qp] *
            (1.0 + _alpha + (1.0 + _alpha) * _zeta[_qp] / _dt) -
        (_alpha + (1.0 + 2.0 * _alpha) * _zeta[_qp] / _dt) * _stress_old[_qp].row(_component) *
            _grad_test[_i][_qp] +
        (_alpha * _zeta[_qp] / _dt) * _stress_older[_qp].row(_component) * _grad_test[_i][_qp];

    if (_volumetric_locking_correction)
      residual += (_stress[_qp].trace() * (1.0 + _alpha + (1.0 + _alpha) * _zeta[_qp] / _dt) -
                   (_alpha + (1.0 + 2.0 * _alpha) * _zeta[_qp] / _dt) * _stress_old[_qp].trace() +
                   (_alpha * _zeta[_qp] / _dt) * _stress_older[_qp].trace()) /
                  3.0 * (_avg_grad_test[_i] - _grad_test[_i][_qp](_component));
  }
  else
    residual = 0.0;

  return residual;
}
