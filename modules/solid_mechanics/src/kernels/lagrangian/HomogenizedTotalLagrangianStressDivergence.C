//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizedTotalLagrangianStressDivergence.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableScalar.h"

registerMooseObject("SolidMechanicsApp", HomogenizedTotalLagrangianStressDivergence);

InputParameters
HomogenizedTotalLagrangianStressDivergence::validParams()
{
  InputParameters params = HomogenizationInterface<TotalLagrangianStressDivergence>::validParams();
  params.addClassDescription("Total Lagrangian stress equilibrium kernel with "
                             "homogenization constraint Jacobian terms");
  params.renameCoupledVar(
      "scalar_variable", "macro_var", "Optional scalar field with the macro gradient");

  return params;
}

HomogenizedTotalLagrangianStressDivergence::HomogenizedTotalLagrangianStressDivergence(
    const InputParameters & parameters)
  : HomogenizationInterface<TotalLagrangianStressDivergence>(parameters)
{
}

std::set<std::string>
HomogenizedTotalLagrangianStressDivergence::additionalROVariables()
{
  // Add the scalar variable to the list of variables that this kernel contributes to
  std::set<std::string> vars = TotalLagrangianStressDivergence::additionalROVariables();
  vars.insert(_kappa_var_ptr->name());
  return vars;
}

void
HomogenizedTotalLagrangianStressDivergence::computeScalarResidual()
{
  if (_alpha != 0)
    return;

  std::vector<Real> scalar_residuals(_k_order);

  // only assemble scalar residual once; i.e. when handling the first displacement component
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpResidual();
    const auto dV = _JxW[_qp] * _coord[_qp];

    // index for residual vector
    unsigned int h = 0;

    for (const auto & [indices, constraint] : cmap())
    {
      const auto [i, j] = indices;
      const auto [ctype, ctarget] = constraint;
      const auto cval = ctarget->value(_t, _q_point[_qp]);

      // value to be constrained
      Real val;
      if (_large_kinematics)
      {
        if (ctype == Homogenization::ConstraintType::Stress)
          val = _pk1[_qp](i, j);
        else if (ctype == Homogenization::ConstraintType::Strain)
          val = _F[_qp](i, j) - (Real(i == j));
        else
          mooseError("Unknown constraint type in the integral!");
      }
      else
      {
        if (ctype == Homogenization::ConstraintType::Stress)
          val = _pk1[_qp](i, j);
        else if (ctype == Homogenization::ConstraintType::Strain)
          val = 0.5 * (_F[_qp](i, j) + _F[_qp](j, i)) - (Real(i == j));
        else
          mooseError("Unknown constraint type in the integral!");
      }

      scalar_residuals[h++] += (val - cval) * dV;
    }
  }

  addResiduals(
      _assembly, scalar_residuals, _kappa_var_ptr->dofIndices(), _kappa_var_ptr->scalingFactor());
}

void
HomogenizedTotalLagrangianStressDivergence::computeScalarJacobian()
{
  if (_alpha != 0)
    return;

  _local_ke.resize(_k_order, _k_order);

  // only assemble scalar residual once; i.e. when handling the first displacement component
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(_kappa_var);
    const auto dV = _JxW[_qp] * _coord[_qp];

    // index for Jacobian row
    unsigned int h = 0;

    for (const auto & [indices1, constraint1] : cmap())
    {
      const auto [i, j] = indices1;
      const auto ctype = constraint1.first;

      // index for Jacobian col
      unsigned int m = 0;

      for (const auto & [indices2, constraint2] : cmap())
      {
        const auto [k, l] = indices2;
        if (ctype == Homogenization::ConstraintType::Stress)
          _local_ke(h, m++) += dV * (_dpk1[_qp](i, j, k, l));
        else if (ctype == Homogenization::ConstraintType::Strain)
        {
          if (_large_kinematics)
            _local_ke(h, m++) += dV * (Real(i == k && j == l));
          else
            _local_ke(h, m++) += dV * (0.5 * Real(i == k && j == l) + 0.5 * Real(i == l && j == k));
        }
        else
          mooseError("Unknown constraint type in Jacobian calculator!");
      }
      h++;
    }
  }

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->scalingFactor());
}

void
HomogenizedTotalLagrangianStressDivergence::computeScalarOffDiagJacobian(
    const unsigned int jvar_num)
{
  // ONLY assemble the contribution from _alpha component, which is connected with _var
  // The other components are handled by other kernel instances with other _alpha
  if (jvar_num != _var.number())
    return;

  const auto & jvar = getVariable(jvar_num);
  const auto jvar_size = jvar.phiSize();
  _local_ke.resize(_k_order, jvar_size);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const auto dV = _JxW[_qp] * _coord[_qp];

    // index for Jacobian row
    unsigned int h = 0;

    for (const auto & [indices, constraint] : cmap())
    {
      std::tie(_m, _n) = indices;
      _ctype = constraint.first;
      initScalarQpOffDiagJacobian(jvar);
      for (_j = 0; _j < jvar_size; _j++)
        _local_ke(h, _j) += dV * computeScalarQpOffDiagJacobian(jvar_num);
      h++;
    }
  }

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              jvar.dofIndices(),
              _kappa_var_ptr->scalingFactor());
}

void
HomogenizedTotalLagrangianStressDivergence::computeOffDiagJacobianScalarLocal(
    const unsigned int svar_num)
{
  // Just in case, skip any other scalar variables
  if (svar_num != _kappa_var)
    return;

  _local_ke.resize(_test.size(), _k_order);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    unsigned int l = 0;
    const auto dV = _JxW[_qp] * _coord[_qp];
    for (const auto & [indices, constraint] : cmap())
    {
      // copy constraint indices to protected variables to pass to Qp routine
      std::tie(_m, _n) = indices;
      _ctype = constraint.first;
      initScalarQpJacobian(svar_num);
      for (_i = 0; _i < _test.size(); _i++)
        _local_ke(_i, l) += dV * computeQpOffDiagJacobianScalar(svar_num);
      l++;
    }
  }

  addJacobian(
      _assembly, _local_ke, _var.dofIndices(), _kappa_var_ptr->dofIndices(), _var.scalingFactor());
}

Real
HomogenizedTotalLagrangianStressDivergence::computeQpOffDiagJacobianScalar(
    unsigned int /*svar_num*/)
{
  return _dpk1[_qp].contractionKl(_m, _n, gradTest(_alpha));
}

Real
HomogenizedTotalLagrangianStressDivergence::computeScalarQpOffDiagJacobian(
    unsigned int /*jvar_num*/)
{
  if (_ctype == Homogenization::ConstraintType::Stress)
    return _dpk1[_qp].contractionIj(_m, _n, gradTrial(_alpha));
  else if (_ctype == Homogenization::ConstraintType::Strain)
    if (_large_kinematics)
      return Real(_m == _alpha) * gradTrial(_alpha)(_m, _n);
    else
      return 0.5 * (Real(_m == _alpha) * gradTrial(_alpha)(_m, _n) +
                    Real(_n == _alpha) * gradTrial(_alpha)(_n, _m));
  else
    mooseError("Unknown constraint type in kernel calculation!");
}
