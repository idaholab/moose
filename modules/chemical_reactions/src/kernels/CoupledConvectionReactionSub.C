/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledConvectionReactionSub.h"

template <>
InputParameters
validParams<CoupledConvectionReactionSub>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("weight", 1.0, "Weight of the equilibrium species");
  params.addParam<Real>("log_k", 0.0, "Equilibrium constant of dissociation equilibrium reaction");
  params.addParam<Real>("sto_u",
                        1.0,
                        "Stoichiometric coef of the primary spceices the kernel "
                        "operates on in the equilibrium reaction");
  params.addRequiredParam<std::vector<Real>>(
      "sto_v",
      "The stoichiometric coefficients of coupled primary species in equilibrium reaction");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addCoupledVar("v", "List of coupled primary species");
  return params;
}

CoupledConvectionReactionSub::CoupledConvectionReactionSub(const InputParameters & parameters)
  : Kernel(parameters),
    _weight(getParam<Real>("weight")),
    _log_k(getParam<Real>("log_k")),
    _sto_u(getParam<Real>("sto_u")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _cond(getMaterialProperty<Real>("conductivity")),
    _grad_p(coupledGradient("p"))
{
  const unsigned int n = coupledComponents("v");
  _vars.resize(n);
  _vals.resize(n);
  _grad_vals.resize(n);

  for (unsigned int i = 0; i < n; ++i)
  {
    _vars[i] = coupled("v", i);
    _vals[i] = &coupledValue("v", i);
    _grad_vals[i] = &coupledGradient("v", i);
  }
}

Real
CoupledConvectionReactionSub::computeQpResidual()
{
  RealGradient darcy_vel = -_grad_p[_qp] * _cond[_qp];
  RealGradient d_u = _sto_u * std::pow(_u[_qp], _sto_u - 1.0) * _grad_u[_qp];
  RealGradient d_var_sum = 0.0;
  const Real d_v_u = std::pow(_u[_qp], _sto_u);

  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    d_u *= std::pow((*_vals[i])[_qp], _sto_v[i]);

    RealGradient d_var =
        d_v_u * _sto_v[i] * std::pow((*_vals[i])[_qp], _sto_v[i] - 1.0) * (*_grad_vals[i])[_qp];
    for (unsigned int j = 0; j < _vals.size(); ++j)
      if (j != i)
        d_var *= std::pow((*_vals[j])[_qp], _sto_v[j]);

    d_var_sum += d_var;
  }

  return _weight * std::pow(10.0, _log_k) * _test[_i][_qp] * darcy_vel * (d_u + d_var_sum);
}

Real
CoupledConvectionReactionSub::computeQpJacobian()
{
  RealGradient darcy_vel = -_grad_p[_qp] * _cond[_qp];
  RealGradient d_u_1 = _sto_u * std::pow(_u[_qp], _sto_u - 1.0) * _grad_phi[_j][_qp];
  RealGradient d_u_2 =
      _phi[_j][_qp] * _sto_u * (_sto_u - 1.0) * std::pow(_u[_qp], _sto_u - 2.0) * _grad_u[_qp];
  RealGradient d_u;
  RealGradient d_var_sum = 0.0;
  const Real d_v_u = _sto_u * std::pow(_u[_qp], _sto_u - 1.0) * _phi[_j][_qp];

  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    d_u_1 *= std::pow((*_vals[i])[_qp], _sto_v[i]);
    d_u_2 *= std::pow((*_vals[i])[_qp], _sto_v[i]);

    RealGradient d_var =
        d_v_u * _sto_v[i] * std::pow((*_vals[i])[_qp], _sto_v[i] - 1.0) * (*_grad_vals[i])[_qp];
    for (unsigned int j = 0; j < _vals.size(); ++j)
      if (j != i)
        d_var *= std::pow((*_vals[j])[_qp], _sto_v[j]);

    d_var_sum += d_var;
  }

  d_u = d_u_1 + d_u_2;
  return _weight * std::pow(10.0, _log_k) * _test[_i][_qp] * darcy_vel * (d_u + d_var_sum);
}

Real
CoupledConvectionReactionSub::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_vals.size() == 0)
    return 0.0;

  RealGradient darcy_vel = -_grad_p[_qp] * _cond[_qp];
  RealGradient diff1 = _sto_u * std::pow(_u[_qp], _sto_u - 1.0) * _grad_u[_qp];
  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    if (jvar == _vars[i])
      diff1 *= _sto_v[i] * std::pow((*_vals[i])[_qp], _sto_v[i] - 1.0) * _phi[_j][_qp];
    else
      diff1 *= std::pow((*_vals[i])[_qp], _sto_v[i]);
  }

  Real val_u = std::pow(_u[_qp], _sto_u);
  RealGradient diff2_1(1.0, 1.0, 1.0);
  RealGradient diff2_2(1.0, 1.0, 1.0);
  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar == _vars[i])
    {
      diff2_1 = _sto_v[i] * (_sto_v[i] - 1.0) * std::pow((*_vals[i])[_qp], _sto_v[i] - 2.0) *
                _phi[_j][_qp] * (*_grad_vals[i])[_qp];
      diff2_2 = _sto_v[i] * std::pow((*_vals[i])[_qp], _sto_v[i] - 1.0) * _grad_phi[_j][_qp];
    }

  RealGradient diff2 = val_u * (diff2_1 + diff2_2);
  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar != _vars[i])
    {
      diff2 *= std::pow((*_vals[i])[_qp], _sto_v[i]);
    }

  RealGradient diff3;
  RealGradient diff3_sum = 0.0;
  Real val_jvar = 0.0;
  unsigned int var = 0;

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar == _vars[i])
    {
      var = i;
      val_jvar = val_u * _sto_v[i] * std::pow((*_vals[i])[_qp], _sto_v[i] - 1.0) * _phi[_j][_qp];
    }

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (i != var)
    {
      diff3 = val_jvar * _sto_v[i] * std::pow((*_vals[i])[_qp], _sto_v[i] - 1.0) *
              (*_grad_vals[i])[_qp];

      for (unsigned int j = 0; j < _vals.size(); ++j)
        if (j != var && j != i)
          diff3 *= std::pow((*_vals[j])[_qp], _sto_v[j]);

      diff3_sum += diff3;
    }

  return _weight * std::pow(10.0, _log_k) * _test[_i][_qp] * darcy_vel *
         (diff1 + diff2 + diff3_sum);
}
