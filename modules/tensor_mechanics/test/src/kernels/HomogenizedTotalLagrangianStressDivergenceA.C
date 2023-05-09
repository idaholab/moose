//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizedTotalLagrangianStressDivergenceA.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableScalar.h"
// #include "Assembly.h"
// #include "MooseVariableFE.h"
// #include "MooseVariableScalar.h"
// #include "SystemBase.h"

// #include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsTestApp", HomogenizedTotalLagrangianStressDivergenceA);

InputParameters
HomogenizedTotalLagrangianStressDivergenceA::validParams()
{
  InputParameters params = TotalLagrangianStressDivergenceS::validParams();
  params.addClassDescription("Total Lagrangian stress equilibrium kernel with "
                             "homogenization constraint Jacobian terms");
  params.renameCoupledVar(
      "scalar_variable", "macro_var", "Optional scalar field with the macro gradient");
  params.addRequiredCoupledVar("macro_other", "Other components of coupled scalar variable");
  params.addRequiredParam<unsigned int>("prime_scalar", "Either 0=_var or 1=_other scalar");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_types",
      HomogenizationA::constraintType,
      "Type of each constraint: strain, stress, or none. The types are specified in the "
      "column-major order, and there must be 9 entries in total.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "targets", "Functions giving the targets to hit for constraint types that are not none.");

  return params;
}

HomogenizedTotalLagrangianStressDivergenceA::HomogenizedTotalLagrangianStressDivergenceA(
    const InputParameters & parameters)
  : TotalLagrangianStressDivergenceS(parameters),
    _beta(getParam<unsigned int>("prime_scalar")),
    _kappao_var_ptr(getScalarVar("macro_other", 0)),
    _kappao_var(coupledScalar("macro_other")),
    _ko_order(getScalarVar("macro_other", 0)->order()),
    _kappa_other(coupledScalarValue("macro_other"))
{
  // Constraint types
  auto types = getParam<MultiMooseEnum>("constraint_types");
  if (types.size() != Moose::dim * Moose::dim)
    mooseError("Number of constraint types must equal dim * dim. ", types.size(), " are provided.");

  // Targets to hit
  const std::vector<FunctionName> & fnames = getParam<std::vector<FunctionName>>("targets");

  // Prepare the constraint map
  unsigned int fcount = 0;
  for (const auto j : make_range(Moose::dim))
    for (const auto i : make_range(Moose::dim))
    {
      const auto idx = i + Moose::dim * j;
      const auto ctype = static_cast<HomogenizationA::ConstraintType>(types.get(idx));
      if (ctype != HomogenizationA::ConstraintType::None)
      {
        const Function * const f = &getFunctionByName(fnames[fcount++]);
        _cmap[{i, j}] = {ctype, f};
      }
    }
}

Real
HomogenizedTotalLagrangianStressDivergenceA::computeQpResidual()
{
  // Assemble R_alpha if _beta==0
  if (_beta == 0)
    return gradTest(_alpha).doubleContraction(_pk1[_qp]);
  else
    return 0.0;
}

Real
HomogenizedTotalLagrangianStressDivergenceA::computeQpJacobianDisplacement(unsigned int alpha,
                                                                           unsigned int beta)
{
  // Assemble J-alpha-beta if _beta==0
  if (_beta == 0)
    return gradTest(alpha).doubleContraction(_dpk1[_qp] * gradTrial(beta));
  else
    return 0.0;
}

void
HomogenizedTotalLagrangianStressDivergenceA::computeScalarResidual()
{
  std::vector<Real> scalar_residuals(_k_order);

  // Assemble R_beta if _alpha==0
  if (_alpha == 0)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpResidual();
      Real dV = _JxW[_qp] * _coord[_qp];
      _h = 0; // single index for residual vector; double indices for constraint tensor component
      for (auto && [indices, constraint] : _cmap)
      {
        auto && [i, j] = indices;
        auto && [ctype, ctarget] = constraint;

        // ONLY the component(s) that this constraint will contribute here;
        // other one is handled in the other constraint
        if (_beta == 0)
        {
          if (_h == 1) // only assemble first=0 component of _hvar, then break the loop
            break;
        }
        else
        {
          // skip the first component (_hvar) and continue to "first" component of _avar
          if (_h == 0)
          {
            _h++;
            continue;
          }
        }

        // I am not great with C++ precedence; so, store the index
        unsigned int r_ind = -_beta + _h; // move 1 row up if _beta=1 for the other scalar
        _h++;                             // increment the index before we forget
        if (_large_kinematics)
        {
          if (ctype == HomogenizationA::ConstraintType::Stress)
            scalar_residuals[r_ind] += dV * (_pk1[_qp](i, j) - ctarget->value(_t, _q_point[_qp]));
          else if (ctype == HomogenizationA::ConstraintType::Strain)
            scalar_residuals[r_ind] +=
                dV * (_F[_qp](i, j) - (Real(i == j) + ctarget->value(_t, _q_point[_qp])));
          else
            mooseError("Unknown constraint type in the integral!");
        }
        else
        {
          if (ctype == HomogenizationA::ConstraintType::Stress)
            scalar_residuals[r_ind] += dV * (_pk1[_qp](i, j) - ctarget->value(_t, _q_point[_qp]));
          else if (ctype == HomogenizationA::ConstraintType::Strain)
            scalar_residuals[r_ind] += dV * (0.5 * (_F[_qp](i, j) + _F[_qp](j, i)) -
                                             (Real(i == j) + ctarget->value(_t, _q_point[_qp])));
          else
            mooseError("Unknown constraint type in the integral!");
        }
      }
    }
  }

  addResiduals(
      _assembly, scalar_residuals, _kappa_var_ptr->dofIndices(), _kappa_var_ptr->scalingFactor());
}

void
HomogenizedTotalLagrangianStressDivergenceA::computeScalarJacobian()
{
  _local_ke.resize(_k_order, _k_order);

  // Assemble J_beta_beta if _alpha==0
  if (_alpha == 0)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpJacobian(_kappa_var);
      Real dV = _JxW[_qp] * _coord[_qp];

      _h = 0;
      for (auto && [indices1, constraint1] : _cmap)
      {
        auto && [i, j] = indices1;
        auto && ctype = constraint1.first;

        // identical logic to computeScalarResidual
        if (_beta == 0)
        {
          if (_h == 1)
            break;
        }
        else
        {
          if (_h == 0)
          {
            _h++;
            continue;
          }
        }

        _l = 0;
        for (auto && indices2 : _cmap)
        {
          auto && [a, b] = indices2.first;

          // identical logic to computeScalarResidual, but for column index
          if (_beta == 0)
          {
            if (_l == 1)
              break;
          }
          else
          {
            if (_l == 0)
            {
              _l++;
              continue;
            }
          }

          unsigned int c_ind = -_beta + _l; // move 1 column left if _beta=1 for the other scalar
          _l++;                             // increment the index before we forget
          if (ctype == HomogenizationA::ConstraintType::Stress)
            _local_ke(-_beta + _h, c_ind) += dV * (_dpk1[_qp](i, j, a, b));
          else if (ctype == HomogenizationA::ConstraintType::Strain)
          {
            if (_large_kinematics)
              _local_ke(-_beta + _h, c_ind) += dV * (Real(i == a && j == b));
            else
              _local_ke(-_beta + _h, c_ind) +=
                  dV * (0.5 * Real(i == a && j == b) + 0.5 * Real(i == b && j == a));
          }
          else
            mooseError("Unknown constraint type in Jacobian calculator!");
        }
        _h++;
      }
    }
  }

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->scalingFactor());
}

void
HomogenizedTotalLagrangianStressDivergenceA::computeScalarOffDiagJacobian(
    const unsigned int jvar_num)
{
  // Assembly J_alpha_beta ONLY
  if (jvar_num == _var.number())
  {
    _local_ke.resize(_k_order, _test.size());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      // single index for Jacobian column; double indices for constraint tensor component
      unsigned int h = 0;
      Real dV = _JxW[_qp] * _coord[_qp];
      for (auto && [indices, constraint] : _cmap)
      {
        // identical logic to computeScalarResidual
        if (_beta == 0)
        {
          if (h == 1)
            break;
        }
        else
        {
          if (h == 0)
          {
            h++;
            continue;
          }
        }
        // copy constraint indices to protected variables to pass to Qp routine
        _m = indices.first;
        _n = indices.second;
        _ctype = constraint.first;
        initScalarQpOffDiagJacobian(_var);
        for (_j = 0; _j < _test.size(); _j++)
          _local_ke(-_beta + h, _j) += dV * computeScalarQpOffDiagJacobian(jvar_num);
        h++;
      }
    }

    addJacobian(_assembly,
                _local_ke,
                _kappa_var_ptr->dofIndices(),
                _var.dofIndices(),
                _kappa_var_ptr->scalingFactor());
  }
}

void
HomogenizedTotalLagrangianStressDivergenceA::computeOffDiagJacobianScalarLocal(
    const unsigned int svar_num)
{
  // Assembly J_beta_alpha ONLY
  if (svar_num == _kappa_var)
  {
    _local_ke.resize(_test.size(), _k_order);

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      // single index for Jacobian row; double indices for constraint tensor component
      unsigned int l = 0;
      Real dV = _JxW[_qp] * _coord[_qp];
      for (auto && [indices, constraint] : _cmap)
      {
        // identical logic to computeScalarResidual, but for column index
        if (_beta == 0)
        {
          if (l == 1)
            break;
        }
        else
        {
          if (l == 0)
          {
            l++;
            continue;
          }
        }
        // copy constraint indices to protected variables to pass to Qp routine
        _m = indices.first;
        _n = indices.second;
        _ctype = constraint.first;
        initScalarQpJacobian(svar_num);
        for (_i = 0; _i < _test.size(); _i++)
        {
          _local_ke(_i, -_beta + l) += dV * computeQpOffDiagJacobianScalar(svar_num);
        }
        l++;
      }
    }

    addJacobian(_assembly,
                _local_ke,
                _var.dofIndices(),
                _kappa_var_ptr->dofIndices(),
                _var.scalingFactor());
  }
}

Real
HomogenizedTotalLagrangianStressDivergenceA::computeQpOffDiagJacobianScalar(unsigned int svar_num)
{
  if (svar_num == _kappa_var)
    return _dpk1[_qp].contractionKl(_m, _n, gradTest(_alpha));
  else
    return 0.;
}

Real
HomogenizedTotalLagrangianStressDivergenceA::computeScalarQpOffDiagJacobian(unsigned int jvar_num)
{
  if (jvar_num == _var.number())
  {
    if (_ctype == HomogenizationA::ConstraintType::Stress)
      return _dpk1[_qp].contractionIj(_m, _n, gradTrial(_alpha));
    else if (_ctype == HomogenizationA::ConstraintType::Strain)
      if (_large_kinematics)
        return Real(_m == _alpha) * gradTrial(_alpha)(_m, _n);
      else
        return 0.5 * (Real(_m == _alpha) * gradTrial(_alpha)(_m, _n) +
                      Real(_n == _alpha) * gradTrial(_alpha)(_n, _m));
    else
      mooseError("Unknown constraint type in kernel calculation!");
  }
  else
    return 0.;
}

void
HomogenizedTotalLagrangianStressDivergenceA::computeScalarOffDiagJacobianScalar(
    const unsigned int svar_num)
{
  // Only do this for the other macro variable
  if (svar_num == _kappao_var)
  {
    _local_ke.resize(_k_order, _ko_order);

    // Assemble J-kappa-kappa_other if _alpha==0
    if (_alpha == 0)
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        initScalarQpJacobian(_kappa_var);
        Real dV = _JxW[_qp] * _coord[_qp];

        _h = 0;
        for (auto && [indices1, constraint1] : _cmap)
        {
          auto && [i, j] = indices1;
          auto && ctype = constraint1.first;

          // identical logic to computeScalarResidual
          if (_beta == 0)
          {
            if (_h == 1)
              break;
          }
          else
          {
            if (_h == 0)
            {
              _h++;
              continue;
            }
          }

          _l = 0;
          for (auto && indices2 : _cmap)
          {
            auto && [a, b] = indices2.first;

            // OPPOSITE logic/scalar from computeScalarResidual, AND for column index
            if (_beta == 1)
            {
              if (_l == 1) // only assemble first=0 component of _hvar, then break the loop
                break;
            }
            else
            {
              if (_l == 0) // skip first component (_hvar) & continue to "first" component of _avar
              {
                _l++;
                continue;
              }
            }

            unsigned int c_ind =
                -(1 - _beta) + _l; // DON'T move 1 column left if _beta=1 for the other scalar
            _l++;
            if (ctype == HomogenizationA::ConstraintType::Stress)
              _local_ke(-_beta + _h, c_ind) += dV * (_dpk1[_qp](i, j, a, b));
            else if (ctype == HomogenizationA::ConstraintType::Strain)
            {
              if (_large_kinematics)
                _local_ke(-_beta + _h, c_ind) += dV * (Real(i == a && j == b));
              else
                _local_ke(-_beta + _h, c_ind) +=
                    dV * (0.5 * Real(i == a && j == b) + 0.5 * Real(i == b && j == a));
            }
            else
              mooseError("Unknown constraint type in Jacobian calculator!");
          }
          _h++;
        }
      }
    }

    addJacobian(_assembly,
                _local_ke,
                _kappa_var_ptr->dofIndices(),
                _kappao_var_ptr->dofIndices(),
                _kappa_var_ptr->scalingFactor());
  }
}
