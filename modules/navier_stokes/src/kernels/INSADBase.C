//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADBase.h"
#include "Assembly.h"

defineADValidParams(
    INSADBase,
    ADKernel,
    params.addClassDescription("This class computes various strong and weak components of the "
                               "incompressible navier stokes equations which can then be assembled "
                               "together in child classes.");
    // Coupled variables
    params.addRequiredCoupledVar("u", "x-velocity");
    params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
    params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D
    params.addRequiredCoupledVar("p", "pressure");
    params.addCoupledVar("temperature",
                         "The temperature on which material properties may depend. If properties "
                         "do depend on temperature, this variable must be coupled in in order to "
                         "correctly resize the element matrix");

    params.addParam<RealVectorValue>("gravity",
                                     RealVectorValue(0, 0, 0),
                                     "Direction of the gravity vector");

    params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
    params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

    params.addParam<Real>("alpha", 1., "Multiplicative factor on the stabilization parameter tau.");
    params.addParam<bool>("laplace",
                          true,
                          "Whether the viscous term of the momentum equations is in laplace form.");
    params.addParam<bool>("convective_term", true, "Whether to include the convective term.");
    params.addParam<bool>("transient_term",
                          false,
                          "Whether there should be a transient term in the momentum residuals."););

template <ComputeStage compute_stage>
INSADBase<compute_stage>::INSADBase(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),

    // Coupled variables
    _u_vel(adCoupledValue("u")),
    _v_vel(adCoupledValue("v")),
    _w_vel(adCoupledValue("w")),
    _p(adCoupledValue("p")),

    // Gradients
    _grad_u_vel(adCoupledGradient("u")),
    _grad_v_vel(adCoupledGradient("v")),
    _grad_w_vel(adCoupledGradient("w")),
    _grad_p(adCoupledGradient("p")),

    // time derivatives
    _u_vel_dot(adCoupledDot("u")),
    _v_vel_dot(adCoupledDot("v")),
    _w_vel_dot(adCoupledDot("w")),

    _gravity(adGetParam<RealVectorValue>("gravity")),

    // Material properties
    _mu(adGetADMaterialProperty<Real>("mu_name")),
    _rho(adGetADMaterialProperty<Real>("rho_name")),

    _alpha(adGetParam<Real>("alpha")),
    _laplace(adGetParam<bool>("laplace")),
    _convective_term(adGetParam<bool>("convective_term")),
    _transient_term(adGetParam<bool>("transient_term"))
{
}

template <ComputeStage compute_stage>
INSADBase<compute_stage>::~INSADBase()
{
  _tau.release();
}

template <ComputeStage compute_stage>
INSVectorValue<compute_stage>
INSADBase<compute_stage>::convectiveTerm()
{
  INSVectorValue<compute_stage> U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return _rho[_qp] * INSVectorValue<compute_stage>(
                         U * _grad_u_vel[_qp], U * _grad_v_vel[_qp], U * _grad_w_vel[_qp]);
}

template <ComputeStage compute_stage>
INSVectorValue<compute_stage>
INSADBase<compute_stage>::weakViscousTermLaplace(unsigned comp)
{
  switch (comp)
  {
    case 0:
      return _mu[_qp] * _grad_u_vel[_qp];

    case 1:
      return _mu[_qp] * _grad_v_vel[_qp];

    case 2:
      return _mu[_qp] * _grad_w_vel[_qp];

    default:
      return adZeroValue()[_qp];
  }
}

template <ComputeStage compute_stage>
INSVectorValue<compute_stage>
INSADBase<compute_stage>::weakViscousTermTraction(unsigned comp)
{
  switch (comp)
  {
    case 0:
    {
      INSVectorValue<compute_stage> transpose(
          _grad_u_vel[_qp](0), _grad_v_vel[_qp](0), _grad_w_vel[_qp](0));
      return _mu[_qp] * _grad_u_vel[_qp] + _mu[_qp] * transpose;
    }

    case 1:
    {
      INSVectorValue<compute_stage> transpose(
          _grad_u_vel[_qp](1), _grad_v_vel[_qp](1), _grad_w_vel[_qp](1));
      return _mu[_qp] * _grad_v_vel[_qp] + _mu[_qp] * transpose;
    }

    case 2:
    {
      INSVectorValue<compute_stage> transpose(
          _grad_u_vel[_qp](2), _grad_v_vel[_qp](2), _grad_w_vel[_qp](2));
      return _mu[_qp] * _grad_w_vel[_qp] + _mu[_qp] * transpose;
    }

    default:
      return adZeroValue()[_qp];
  }
}

template <ComputeStage compute_stage>
INSVectorValue<compute_stage>
INSADBase<compute_stage>::strongPressureTerm()
{
  return _grad_p[_qp];
}

template <ComputeStage compute_stage>
INSReal<compute_stage>
INSADBase<compute_stage>::weakPressureTerm()
{
  return -_p[_qp];
}

template <ComputeStage compute_stage>
INSVectorValue<compute_stage>
INSADBase<compute_stage>::gravityTerm()
{
  return -_rho[_qp] * _gravity;
}

template <ComputeStage compute_stage>
INSVectorValue<compute_stage>
INSADBase<compute_stage>::timeDerivativeTerm()
{
  return _rho[_qp] *
         INSVectorValue<compute_stage>(_u_vel_dot[_qp], _v_vel_dot[_qp], _w_vel_dot[_qp]);
}

template <ComputeStage compute_stage>
void
INSADBase<compute_stage>::computeHMax()
{
  _hmax = _current_elem->hmax();
}

template <>
void
INSADBase<JACOBIAN>::computeHMax()
{
  _hmax = 0;

  for (unsigned int n_outer = 0; n_outer < _current_elem->n_vertices(); n_outer++)
    for (unsigned int n_inner = n_outer + 1; n_inner < _current_elem->n_vertices(); n_inner++)
    {
      VectorValue<ADReal> diff = (_current_elem->point(n_outer) - _current_elem->point(n_inner));
      unsigned dimension = 0;
      for (const auto & disp_num : _displacements)
      {
        diff(dimension).derivatives()[disp_num * _sys.getMaxVarNDofsPerElem() + n_outer] = 1.;
        diff(dimension++).derivatives()[disp_num * _sys.getMaxVarNDofsPerElem() + n_inner] = -1.;
      }

      _hmax = std::max(_hmax, diff.norm_sq());
    }

  _hmax = std::sqrt(_hmax);
}

template <ComputeStage compute_stage>
INSReal<compute_stage>
INSADBase<compute_stage>::tau()
{
  auto && nu = _mu[_qp] / _rho[_qp];
  INSVectorValue<compute_stage> U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  auto && transient_part = _transient_term ? 4. / (_dt * _dt) : 0.;
  return _alpha / std::sqrt(transient_part + (2. * U.norm() / _hmax) * (2. * U.norm() / _hmax) +
                            9. * (4. * nu / (_hmax * _hmax)) * (4. * nu / (_hmax * _hmax)));
}

template <ComputeStage compute_stage>
INSReal<compute_stage>
INSADBase<compute_stage>::tauNodal()
{
  auto nu = _mu[_qp] / _rho[_qp];
  INSVectorValue<compute_stage> U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  const auto & h = _current_elem->hmax();
  INSReal<compute_stage> xi;
  if (nu < std::numeric_limits<Real>::epsilon())
    xi = 1;
  else
  {
    auto alpha = U.norm() * h / (2 * nu);
    xi = 1. / std::tanh(alpha) - 1. / alpha;
  }
  return h / (2 * U.norm()) * xi;
}

template class INSADBase<RESIDUAL>;
template class INSADBase<JACOBIAN>;
