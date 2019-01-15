//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumBase.h"
#include "Function.h"

registerADMooseObject("NavierStokesApp", INSADMomentumBase);

defineADValidParams(
    INSADMomentumBase,
    INSADBase,
    params.addRequiredParam<unsigned>("component",
                                      "The velocity component that this is applied to.");
    params.addParam<bool>("integrate_p_by_parts",
                          true,
                          "Whether to integrate the pressure term by parts.");
    params.addParam<bool>("supg",
                          false,
                          "Whether to perform SUPG stabilization of the momentum residuals");
    params.addParam<FunctionName>("forcing_func", 0, "The mms forcing function."););

template <ComputeStage compute_stage>
INSADMomentumBase<compute_stage>::INSADMomentumBase(const InputParameters & parameters)
  : INSADBase<compute_stage>(parameters),
    _component(adGetParam<unsigned>("component")),
    _integrate_p_by_parts(adGetParam<bool>("integrate_p_by_parts")),
    _supg(adGetParam<bool>("supg")),
    _ffn(this->getFunction("forcing_func"))
{
  if (_supg && !_convective_term)
    mooseError("It doesn't make sense to conduct SUPG stabilization without a convective term.");
}

template <ComputeStage compute_stage>
void
INSADMomentumBase<compute_stage>::beforeQpLoop()
{
  if (_supg)
    this->computeHMax();
}

template <ComputeStage compute_stage>
void
INSADMomentumBase<compute_stage>::beforeTestLoop()
{
  computeTestTerms();
  computeGradTestTerms();
  computeGradTestComponentTerms();
}

template <ComputeStage compute_stage>
void
INSADMomentumBase<compute_stage>::computeTestTerms()
{
  _test_terms = -_rho[_qp] * _gravity(_component) - _ffn.value(_t, _q_point[_qp]);
  if (!_integrate_p_by_parts)
    _test_terms += _grad_p[_qp](_component);
  if (_convective_term)
  {
    auto convective_term = _rho[_qp];
    switch (_component)
    {
      case 0:
        convective_term *=
            INSVectorValue<compute_stage>(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) * _grad_u_vel[_qp];
        break;
      case 1:
        convective_term *=
            INSVectorValue<compute_stage>(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) * _grad_v_vel[_qp];
        break;
      case 2:
        convective_term *=
            INSVectorValue<compute_stage>(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) * _grad_w_vel[_qp];
        break;
      default:
        mooseError("_component must be either 0, 1, or 2");
        break;
    }
    _test_terms += convective_term;
  }
}

template <ComputeStage compute_stage>
void
INSADMomentumBase<compute_stage>::computeGradTestTerms()
{
  _grad_test_terms = _mu[_qp] * _grad_u[_qp];
  if (_supg)
  {
    INSVectorValue<compute_stage> U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    auto temp = -_rho[_qp] * _gravity(_component) + _grad_p[_qp](_component) -
                _ffn.value(_t, _q_point[_qp]);
    if (_convective_term)
    {
      auto convective_term = _rho[_qp];
      switch (_component)
      {
        case 0:
          convective_term *= INSVectorValue<compute_stage>(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) *
                             _grad_u_vel[_qp];
          break;
        case 1:
          convective_term *= INSVectorValue<compute_stage>(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) *
                             _grad_v_vel[_qp];
          break;
        case 2:
          convective_term *= INSVectorValue<compute_stage>(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) *
                             _grad_w_vel[_qp];
          break;
        default:
          mooseError("_component must be either 0, 1, or 2");
          break;
      }
      temp += convective_term;
    }
    if (_transient_term)
    {
      auto transient_term = _rho[_qp];
      switch (_component)
      {
        case 0:
          transient_term *= _u_vel_dot[_qp];
          break;
        case 1:
          transient_term *= _v_vel_dot[_qp];
          break;
        case 2:
          transient_term *= _w_vel_dot[_qp];
          break;
        default:
          mooseError("_component must be either 0, 1, or 2");
          break;
      }
      temp += transient_term;
    }
    _grad_test_terms += temp * this->tau() * U;
  }
}

template <ComputeStage compute_stage>
void
INSADMomentumBase<compute_stage>::computeGradTestComponentTerms()
{
  if (_integrate_p_by_parts)
    _grad_test_component_terms = this->weakPressureTerm();
  else
    _grad_test_component_terms = 0;
}

template <ComputeStage compute_stage>
ADResidual
INSADMomentumBase<compute_stage>::computeQpResidual()
{
  return _test[_i][_qp] * _test_terms + _grad_test[_i][_qp] * _grad_test_terms +
         _grad_test[_i][_qp](_component) * _grad_test_component_terms;
}

template class INSADMomentumBase<RESIDUAL>;
template class INSADMomentumBase<JACOBIAN>;
