//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEFrictionSource.h"

registerMooseObject("ShallowWaterApp", SWEFrictionSource);

InputParameters
SWEFrictionSource::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Manning friction source for SWE momentum equations.");
  params.addRequiredCoupledVar("gravity", "Scalar gravity field g");
  params.addRequiredParam<Real>("manning_n", "Manning roughness coefficient n");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  params.addParam<Real>("speed_eps", 1e-12, "Regularization for speed in friction term");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredCoupledVar("hu", "Conserved variable: h*u");
  params.addRequiredCoupledVar("hv", "Conserved variable: h*v");
  return params;
}

SWEFrictionSource::SWEFrictionSource(const InputParameters & parameters)
  : Kernel(parameters),
    _g(coupledValue("gravity")),
    _n_manning(getParam<Real>("manning_n")),
    _h_eps(getParam<Real>("dry_depth")),
    _s_eps(getParam<Real>("speed_eps")),
    _h_var(coupled("h")),
    _hu_var(coupled("hu")),
    _hv_var(coupled("hv")),
    _h(coupledValue("h")),
    _hu(coupledValue("hu")),
    _hv(coupledValue("hv"))
{
}

SWEFrictionSource::~SWEFrictionSource() {}

Real
SWEFrictionSource::computeQpResidual()
{
  const Real h = std::max(_h[_qp], 0.0);
  if (h <= _h_eps)
    return 0.0;
  const Real invh = 1.0 / h;
  const Real u = _hu[_qp] * invh;
  const Real v = _hv[_qp] * invh;
  const Real s = std::sqrt(u * u + v * v + _s_eps * _s_eps);

  // Determine component based on variable
  const bool xmom = (_var.number() == _hu_var);
  const Real comp = xmom ? u : v;
  const Real S = -_g[_qp] * _n_manning * _n_manning * comp * s / std::pow(h, 1.0 / 3.0);
  return S * _test[_i][_qp];
}

Real
SWEFrictionSource::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
SWEFrictionSource::computeQpOffDiagJacobian(unsigned int jvar)
{
  const Real h = std::max(_h[_qp], 0.0);
  if (h <= _h_eps)
    return 0.0;

  const Real invh = 1.0 / h;
  const Real u = _hu[_qp] * invh;
  const Real v = _hv[_qp] * invh;
  const Real s = std::sqrt(u * u + v * v + _s_eps * _s_eps);
  const bool xmom = (_var.number() == _hu_var);

  // Common prefactor
  const Real C = _g[_qp] * _n_manning * _n_manning / std::pow(h, 1.0 / 3.0);

  Real dS = 0.0;
  if (jvar == _hu_var)
  {
    if (xmom)
    {
      // d/dhu of (-g n^2 u s h^{-1/3})
      const Real du_dhu = invh;
      const Real ds_dhu = (u / s) * invh;
      dS = -C * (du_dhu * s + u * ds_dhu);
    }
    else
    {
      // d/dhu of (-g n^2 v s h^{-1/3})
      const Real ds_dhu = (u / s) * invh;
      dS = -C * (0.0 + v * ds_dhu);
    }
    return dS * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _hv_var)
  {
    if (xmom)
    {
      const Real ds_dhv = (v / s) * invh;
      dS = -C * (0.0 + u * ds_dhv);
    }
    else
    {
      const Real dv_dhv = invh;
      const Real ds_dhv = (v / s) * invh;
      dS = -C * (dv_dhv * s + v * ds_dhv);
    }
    return dS * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _h_var)
  {
    // d/dh accounting for u,v dependence and h^{-1/3}; approximate derivative of s
    const Real du_dh = -u * invh;
    const Real dv_dh = -v * invh;
    const Real ds_dh = (u / s) * du_dh + (v / s) * dv_dh; // ~ -s/h
    const Real d_h_pow = (-1.0 / 3.0) * std::pow(h, -4.0 / 3.0);
    if (xmom)
      dS = -_g[_qp] * _n_manning * _n_manning * (du_dh * s + u * ds_dh) * std::pow(h, -1.0 / 3.0) +
           (-_g[_qp] * _n_manning * _n_manning * u * s) * d_h_pow;
    else
      dS = -_g[_qp] * _n_manning * _n_manning * (dv_dh * s + v * ds_dh) * std::pow(h, -1.0 / 3.0) +
           (-_g[_qp] * _n_manning * _n_manning * v * s) * d_h_pow;
    return dS * _phi[_j][_qp] * _test[_i][_qp];
  }

  return 0.0;
}
