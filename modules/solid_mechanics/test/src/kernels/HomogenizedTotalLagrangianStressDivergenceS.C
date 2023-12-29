//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizedTotalLagrangianStressDivergenceS.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableScalar.h"

registerMooseObject("TensorMechanicsTestApp", HomogenizedTotalLagrangianStressDivergenceS);

InputParameters
HomogenizedTotalLagrangianStressDivergenceS::validParams()
{
  InputParameters params = TotalLagrangianStressDivergenceS::validParams();
  params.addClassDescription("Total Lagrangian stress equilibrium kernel with "
                             "homogenization constraint Jacobian terms");
  params.renameCoupledVar(
      "scalar_variable", "macro_var", "Optional scalar field with the macro gradient");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_types",
      HomogenizationS::constraintType,
      "Type of each constraint: strain, stress, or none. The types are specified in the "
      "column-major order, and there must be 9 entries in total.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "targets", "Functions giving the targets to hit for constraint types that are not none.");

  return params;
}

HomogenizedTotalLagrangianStressDivergenceS::HomogenizedTotalLagrangianStressDivergenceS(
    const InputParameters & parameters)
  : TotalLagrangianStressDivergenceS(parameters)
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
      const auto ctype = static_cast<HomogenizationS::ConstraintType>(types.get(idx));
      if (ctype != HomogenizationS::ConstraintType::None)
      {
        const Function * const f = &getFunctionByName(fnames[fcount++]);
        _cmap[{i, j}] = {ctype, f};
      }
    }
}

void
HomogenizedTotalLagrangianStressDivergenceS::computeScalarResidual()
{
  std::vector<Real> scalar_residuals(_k_order);

  // only assemble scalar residual once; i.e. when handling the first displacement component
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

        if (_large_kinematics)
        {
          if (ctype == HomogenizationS::ConstraintType::Stress)
            scalar_residuals[_h++] += dV * (_pk1[_qp](i, j) - ctarget->value(_t, _q_point[_qp]));
          else if (ctype == HomogenizationS::ConstraintType::Strain)
            scalar_residuals[_h++] +=
                dV * (_F[_qp](i, j) - (Real(i == j) + ctarget->value(_t, _q_point[_qp])));
          else
            mooseError("Unknown constraint type in the integral!");
        }
        else
        {
          if (ctype == HomogenizationS::ConstraintType::Stress)
            scalar_residuals[_h++] += dV * (_pk1[_qp](i, j) - ctarget->value(_t, _q_point[_qp]));
          else if (ctype == HomogenizationS::ConstraintType::Strain)
            scalar_residuals[_h++] += dV * (0.5 * (_F[_qp](i, j) + _F[_qp](j, i)) -
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
HomogenizedTotalLagrangianStressDivergenceS::computeScalarJacobian()
{
  _local_ke.resize(_k_order, _k_order);

  // only assemble scalar residual once; i.e. when handling the first displacement component
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
        _l = 0;
        for (auto && indices2 : _cmap)
        {
          auto && [a, b] = indices2.first;
          if (ctype == HomogenizationS::ConstraintType::Stress)
            _local_ke(_h, _l++) += dV * (_dpk1[_qp](i, j, a, b));
          else if (ctype == HomogenizationS::ConstraintType::Strain)
          {
            if (_large_kinematics)
              _local_ke(_h, _l++) += dV * (Real(i == a && j == b));
            else
              _local_ke(_h, _l++) +=
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
HomogenizedTotalLagrangianStressDivergenceS::computeScalarOffDiagJacobian(
    const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);
  // Get dofs and order of this variable; at least one will be _var
  const auto jvar_size = jvar.phiSize();
  _local_ke.resize(_k_order, jvar_size);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // single index for Jacobian column; double indices for constraint tensor component
    unsigned int h = 0;
    Real dV = _JxW[_qp] * _coord[_qp];
    for (auto && [indices, constraint] : _cmap)
    {
      // copy constraint indices to protected variables to pass to Qp routine
      _m = indices.first;
      _n = indices.second;
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
HomogenizedTotalLagrangianStressDivergenceS::computeOffDiagJacobianScalarLocal(
    const unsigned int svar_num)
{

  // Get dofs and order of this scalar; at least one will be _kappa_var
  const auto & svar = _sys.getScalarVariable(_tid, svar_num);
  const unsigned int s_order = svar.order();
  _local_ke.resize(_test.size(), s_order);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    unsigned int l = 0;
    Real dV = _JxW[_qp] * _coord[_qp];
    for (auto && [indices, constraint] : _cmap)
    {
      // copy constraint indices to protected variables to pass to Qp routine
      _m = indices.first;
      _n = indices.second;
      _ctype = constraint.first;
      initScalarQpJacobian(svar_num);
      for (_i = 0; _i < _test.size(); _i++)
      {
        _local_ke(_i, l) += dV * computeQpOffDiagJacobianScalar(svar_num);
      }
      l++;
    }
  }

  addJacobian(_assembly, _local_ke, _var.dofIndices(), svar.dofIndices(), _var.scalingFactor());
}

Real
HomogenizedTotalLagrangianStressDivergenceS::computeQpOffDiagJacobianScalar(unsigned int svar_num)
{
  // Just in case, skip any other scalar variables
  if (svar_num == _kappa_var)
    return _dpk1[_qp].contractionKl(_m, _n, gradTest(_alpha));
  else
    return 0.;
}

Real
HomogenizedTotalLagrangianStressDivergenceS::computeScalarQpOffDiagJacobian(unsigned int jvar_num)
{
  // ONLY assemble the contribution from _alpha component, which is connected with _var
  // The other components are handled by other kernel instances with other _alpha
  if (jvar_num == _var.number())
  {
    if (_ctype == HomogenizationS::ConstraintType::Stress)
      return _dpk1[_qp].contractionIj(_m, _n, gradTrial(_alpha));
    else if (_ctype == HomogenizationS::ConstraintType::Strain)
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
