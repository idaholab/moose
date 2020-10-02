//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledConvectionReactionSub.h"

registerMooseObject("ChemicalReactionsApp", CoupledConvectionReactionSub);

InputParameters
CoupledConvectionReactionSub::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>("weight", 1.0, "Weight of the equilibrium species");
  params.addCoupledVar("log_k", 0.0, "Equilibrium constant of dissociation equilibrium reaction");
  params.addParam<Real>("sto_u",
                        1.0,
                        "Stoichiometric coef of the primary species the kernel "
                        "operates on in the equilibrium reaction");
  params.addCoupledVar(
      "gamma_u", 1.0, "Activity coefficient of primary species that this kernel operates on");
  params.addParam<std::vector<Real>>(
      "sto_v",
      "The stoichiometric coefficients of coupled primary species in equilibrium reaction");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addCoupledVar("v", "List of coupled primary species");
  params.addCoupledVar("gamma_v", 1.0, "Activity coefficients of coupled primary species");
  params.addCoupledVar("gamma_eq", 1.0, "Activity coefficient of this equilibrium species");
  RealVectorValue g(0, 0, 0);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector (default is (0, 0, 0))");
  params.addClassDescription("Convection of equilibrium species");
  return params;
}

CoupledConvectionReactionSub::CoupledConvectionReactionSub(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _weight(getParam<Real>("weight")),
    _log_k(coupledValue("log_k")),
    _sto_u(getParam<Real>("sto_u")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _cond(getMaterialProperty<Real>("conductivity")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _density(getDefaultMaterialProperty<Real>("density")),
    _grad_p(coupledGradient("p")),
    _pvar(coupled("p")),
    _vars(coupledIndices("v")),
    _vals(coupledValues("v")),
    _grad_vals(coupledGradients("v")),
    _gamma_u(coupledValue("gamma_u")),
    _gamma_v(isCoupled("gamma_v")
                 ? coupledValues("gamma_v") // have value
                 : std::vector<const VariableValue *>(coupledComponents("v"),
                                                      &coupledValue("gamma_v"))), // default
    _gamma_eq(coupledValue("gamma_eq"))
{
  const unsigned int n = coupledComponents("v");

  // Check that the correct number of coupled values have been provided
  if (_sto_v.size() != n)
    mooseError("The number of stoichiometric coefficients in sto_v is not equal to the number of "
               "coupled species in ",
               _name);

  if (isCoupled("gamma_v"))
    if (coupledComponents("gamma_v") != n)
      mooseError("The number of activity coefficients in gamma_v is not equal to the number of "
                 "coupled species in ",
                 _name);
}

Real
CoupledConvectionReactionSub::computeQpResidual()
{
  RealVectorValue darcy_vel = -_cond[_qp] * (_grad_p[_qp] - _density[_qp] * _gravity);
  RealGradient d_u =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _grad_u[_qp];
  RealGradient d_var_sum(0.0, 0.0, 0.0);
  const Real d_v_u = std::pow(_gamma_u[_qp] * _u[_qp], _sto_u);

  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    d_u *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);

    RealGradient d_var = d_v_u * _sto_v[i] * (*_gamma_v[i])[_qp] *
                         std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
                         (*_grad_vals[i])[_qp];

    for (unsigned int j = 0; j < _vals.size(); ++j)
      if (j != i)
        d_var *= std::pow((*_gamma_v[i])[_qp] * (*_vals[j])[_qp], _sto_v[j]);

    d_var_sum += d_var;
  }

  mooseAssert(_gamma_eq[_qp] > 0.0, "Activity coefficient must be greater than zero");
  return _weight * std::pow(10.0, _log_k[_qp]) * _test[_i][_qp] * darcy_vel * (d_u + d_var_sum) /
         _gamma_eq[_qp];
}

Real
CoupledConvectionReactionSub::computeQpJacobian()
{
  RealVectorValue darcy_vel = -_cond[_qp] * (_grad_p[_qp] - _density[_qp] * _gravity);

  RealGradient d_u_1 =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _grad_phi[_j][_qp];
  RealGradient d_u_2 = _phi[_j][_qp] * _sto_u * (_sto_u - 1.0) * _gamma_u[_qp] * _gamma_u[_qp] *
                       std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 2.0) * _grad_u[_qp];

  RealGradient d_var_sum(0.0, 0.0, 0.0);
  const Real d_v_u =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _phi[_j][_qp];

  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    d_u_1 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);
    d_u_2 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);

    RealGradient d_var = d_v_u * _sto_v[i] * (*_gamma_v[i])[_qp] *
                         std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
                         (*_grad_vals[i])[_qp];
    for (unsigned int j = 0; j < _vals.size(); ++j)
      if (j != i)
        d_var *= std::pow((*_gamma_v[i])[_qp] * (*_vals[j])[_qp], _sto_v[j]);

    d_var_sum += d_var;
  }

  RealGradient d_u_j = d_u_1 + d_u_2;
  return _weight * std::pow(10.0, _log_k[_qp]) * _test[_i][_qp] * darcy_vel * (d_u_j + d_var_sum) /
         _gamma_eq[_qp];
}

Real
CoupledConvectionReactionSub::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _pvar)
  {
    RealVectorValue ddarcy_vel_dp = -_cond[_qp] * _grad_phi[_j][_qp];

    RealGradient d_u =
        _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _grad_u[_qp];
    RealGradient d_var_sum(0.0, 0.0, 0.0);
    const Real d_v_u = std::pow(_gamma_u[_qp] * _u[_qp], _sto_u);

    for (unsigned int i = 0; i < _vals.size(); ++i)
    {
      d_u *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);

      RealGradient d_var = d_v_u * _sto_v[i] * (*_gamma_v[i])[_qp] *
                           std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
                           (*_grad_vals[i])[_qp];
      for (unsigned int j = 0; j < _vals.size(); ++j)
        if (j != i)
          d_var *= std::pow((*_gamma_v[i])[_qp] * (*_vals[j])[_qp], _sto_v[j]);

      d_var_sum += d_var;
    }
    return _weight * std::pow(10.0, _log_k[_qp]) * _test[_i][_qp] * ddarcy_vel_dp *
           (d_u + d_var_sum) / _gamma_eq[_qp];
  }

  if (_vals.size() == 0)
    return 0.0;

  RealVectorValue darcy_vel = -_cond[_qp] * (_grad_p[_qp] - _density[_qp] * _gravity);
  RealGradient diff1 =
      _sto_u * _gamma_u[_qp] * std::pow(_gamma_u[_qp] * _u[_qp], _sto_u - 1.0) * _grad_u[_qp];
  for (unsigned int i = 0; i < _vals.size(); ++i)
  {
    if (jvar == _vars[i])
      diff1 *= _sto_v[i] * (*_gamma_v[i])[_qp] *
               std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) * _phi[_j][_qp];
    else
      diff1 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);
  }

  Real val_u = std::pow(_gamma_u[_qp] * _u[_qp], _sto_u);
  RealGradient diff2_1(1.0, 1.0, 1.0);
  RealGradient diff2_2(1.0, 1.0, 1.0);
  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar == _vars[i])
    {
      diff2_1 = _sto_v[i] * (*_gamma_v[i])[_qp] * (*_gamma_v[i])[_qp] * (_sto_v[i] - 1.0) *
                std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 2.0) * _phi[_j][_qp] *
                (*_grad_vals[i])[_qp];
      diff2_2 = _sto_v[i] * (*_gamma_v[i])[_qp] *
                std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
                _grad_phi[_j][_qp];
    }

  RealGradient diff2 = val_u * (diff2_1 + diff2_2);
  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar != _vars[i])
    {
      diff2 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i]);
    }

  RealGradient diff3;
  RealGradient diff3_sum(0.0, 0.0, 0.0);
  Real val_jvar = 0.0;
  unsigned int var = 0;

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (jvar == _vars[i])
    {
      var = i;
      val_jvar = val_u * _sto_v[i] * (*_gamma_v[i])[_qp] *
                 std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) * _phi[_j][_qp];
    }

  for (unsigned int i = 0; i < _vals.size(); ++i)
    if (i != var)
    {
      diff3 = val_jvar * _sto_v[i] * (*_gamma_v[i])[_qp] *
              std::pow((*_gamma_v[i])[_qp] * (*_vals[i])[_qp], _sto_v[i] - 1.0) *
              (*_grad_vals[i])[_qp];

      for (unsigned int j = 0; j < _vals.size(); ++j)
        if (j != var && j != i)
          diff3 *= std::pow((*_gamma_v[i])[_qp] * (*_vals[j])[_qp], _sto_v[j]);

      diff3_sum += diff3;
    }

  return _weight * std::pow(10.0, _log_k[_qp]) * _test[_i][_qp] * darcy_vel *
         (diff1 + diff2 + diff3_sum) / _gamma_eq[_qp];
}
