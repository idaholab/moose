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

  params.addParam<bool>(
      "off_diagonal_jacobian", true, "Include the off diagonal parts of the constraint Jacobian");

  return params;
}

HomogenizedTotalLagrangianStressDivergence::HomogenizedTotalLagrangianStressDivergence(
    const InputParameters & parameters)
  : HomogenizationInterface<TotalLagrangianStressDivergence>(parameters),
    _use_off_diagonal(getParam<bool>("off_diagonal_jacobian"))
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
          // Macro_grad ↔ macro_grad: scalar perturbation bypasses F-bar (the
          // homogenization material adds to `_F` AFTER F-bar runs), so use the
          // bypass variant of pk1_jacobian.
          _local_ke(h, m++) += dV * (_dpk1_bypass_fbar[_qp](i, j, k, l));
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
  if (!_use_off_diagonal)
    return;

  // ONLY assemble the contribution from _alpha component, which is connected with _var
  // The other components are handled by other kernel instances with other _alpha
  if (jvar_num != _var.number())
    return;

  const auto & jvar = getVariable(jvar_num);
  const auto jvar_size = jvar.phiSize();
  _local_ke.resize(_k_order, jvar_size);

  // The scalar↔disp Jacobian needs `_avg_grad_trial[_alpha]` populated (for the
  // non-local F-bar chain via `_d_F_stab_d_F_avg · δF_avg`). The base
  // `precalculateOffDiagJacobian` only does this when the off-diag jvar IS a
  // displacement; for the scalar-driven path it must be triggered explicitly.
  if (_stabilize_strain)
  {
    _fe_problem.prepareShapes(jvar_num, _tid);
    _avg_grad_trial[_alpha].resize(_phi.size());
    precalculateJacobianDisplacement(_alpha);
  }

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
  if (!_use_off_diagonal)
    return;

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
  // d(disp residual) / d(scalar_{m,n}) = ∫ gradTest_α : d(PK1)/d(scalar_{m,n}) dV.
  // The macro_gradient adds to `_F` AFTER F-bar runs (in
  // `ComputeLagrangianStrainBase::computeQpProperties`), so scalar perturbations
  // bypass F-bar's chain — use `_dpk1_bypass_fbar` (pk1_jacobian with the F-bar
  // `_d_F_stab_d_F_ust` factor REPLACED by identity in the σ chain).
  return _dpk1_bypass_fbar[_qp].contractionKl(_m, _n, gradTest(_alpha));
}

Real
HomogenizedTotalLagrangianStressDivergence::computeScalarQpOffDiagJacobian(
    unsigned int /*jvar_num*/)
{
  if (_ctype == Homogenization::ConstraintType::Stress)
  {
    // d(PK1_{m,n})/d(grad u_β,j) — local chain via _dpk1 (= dPK1/d(grad u) including
    // local F-bar effect via the σ-chain through `_d_F_stab_d_F_ust`).
    Real J = _dpk1[_qp].contractionIj(_m, _n, gradTrial(_alpha));

    // Non-local F-bar contribution to PK1 component (m, n) via the shared helper —
    // same chain as the regular TL displacement Jacobian but contracted into the single
    // (m, n) entry rather than doubled with gradTest. Guarded on `_stabilize_strain`
    // because `_avg_grad_trial` is only populated when F-bar is on.
    if (_stabilize_strain)
    {
      const RankTwoTensor delta_F_avg = _d_F_d_grad_u[_qp] * _avg_grad_trial[_alpha][_j];
      J += deltaPK1NonLocalFBar(delta_F_avg)(_m, _n);
    }
    return J;
  }
  else if (_ctype == Homogenization::ConstraintType::Strain)
  {
    // d(F_stab_{m,n})/d(disp_α_j) — for F-bar on, the F-bar chain couples F_stab to
    // F_ust through both LOCAL (`_d_F_stab_d_F_ust`) and NON-LOCAL
    // (`_d_F_stab_d_F_avg · δF_avg`) routes. The old `Real(_m == _alpha) *
    // gradTrial(_m, _n)` form captured only the F-bar-off case correctly.
    const RankTwoTensor delta_F_ust_local = _d_F_d_grad_u[_qp] * gradTrialUnstabilized(_alpha);
    RankTwoTensor delta_F_stab = _d_F_stab_d_F_ust[_qp] * delta_F_ust_local;
    if (_stabilize_strain)
    {
      const RankTwoTensor delta_F_avg = _d_F_d_grad_u[_qp] * _avg_grad_trial[_alpha][_j];
      delta_F_stab += _d_F_stab_d_F_avg[_qp] * delta_F_avg;
    }
    if (_large_kinematics)
      return delta_F_stab(_m, _n);
    else
      return 0.5 * (delta_F_stab(_m, _n) + delta_F_stab(_n, _m));
  }
  else
    mooseError("Unknown constraint type in kernel calculation!");
}
