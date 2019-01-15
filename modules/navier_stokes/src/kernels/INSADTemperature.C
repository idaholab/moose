//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADTemperature.h"

registerADMooseObject("NavierStokesApp", INSADTemperature);

defineADValidParams(INSADTemperature,
                    INSADBase,
                    params.addClassDescription(
                        "This class computes the residual and Jacobian contributions for the "
                        "incompressible Navier-Stokes temperature (energy) equation.");

                    params.addParam<MaterialPropertyName>("k_name",
                                                          "k",
                                                          "thermal conductivity name");
                    params.addParam<MaterialPropertyName>("cp_name", "cp", "specific heat name");
                    params.addParam<MaterialPropertyName>("grad_k", 0, "The gradient of k");
                    params.addParam<bool>("supg", false, "Whether to perform SUPG stabilization"););

template <ComputeStage compute_stage>
INSADTemperature<compute_stage>::INSADTemperature(const InputParameters & parameters)
  : INSADBase<compute_stage>(parameters),

    // Material Properties
    _k(adGetADMaterialProperty<Real>("k_name")),
    _cp(adGetADMaterialProperty<Real>("cp_name")),
    _grad_k(adGetADMaterialProperty<RealVectorValue>("grad_k")),
    _u_dot(_var.template adUDot<compute_stage>()),
    _supg(adGetParam<bool>("supg"))
{
}

template <ComputeStage compute_stage>
void
INSADTemperature<compute_stage>::beforeQpLoop()
{
  if (_supg)
    this->computeHMax();
}

template <ComputeStage compute_stage>
void
INSADTemperature<compute_stage>::beforeTestLoop()
{
  _U(0) = _u_vel[_qp];
  _U(1) = _v_vel[_qp];
  _U(2) = _w_vel[_qp];
  computeTestTerms();
  computeGradTestTerms();
}

template <ComputeStage compute_stage>
void
INSADTemperature<compute_stage>::computeTestTerms()
{
  _strong_convective_part = _rho[_qp] * _cp[_qp] * _U * _grad_u[_qp];
  _test_terms = _strong_convective_part;
}

template <ComputeStage compute_stage>
void
INSADTemperature<compute_stage>::computeGradTestTerms()
{
  _grad_test_terms = _k[_qp] * _grad_u[_qp];
  if (_supg)
  {
    auto tau = this->tau();
    _grad_test_terms += _strong_convective_part * tau * _U;
    if (_transient_term)
      _grad_test_terms += _rho[_qp] * _cp[_qp] * _u_dot[_qp] * tau * _U;
  }
}

template <ComputeStage compute_stage>
ADResidual
INSADTemperature<compute_stage>::computeQpResidual()
{
  return _test[_i][_qp] * _test_terms + _grad_test[_i][_qp] * _grad_test_terms;
}

template class INSADTemperature<RESIDUAL>;
template class INSADTemperature<JACOBIAN>;
