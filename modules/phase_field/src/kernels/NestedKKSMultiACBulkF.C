//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedKKSMultiACBulkF.h"

registerMooseObject("PhaseFieldApp", NestedKKSMultiACBulkF);

InputParameters
NestedKKSMultiACBulkF::validParams()
{
  InputParameters params = KKSMultiACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 1 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms NOT dependent on chemical potential.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc.");
  params.addRequiredCoupledVar("all_etas", "Order parameters.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. They must have the same order as Fj_names and global_cs, for "
      "example, c1, c2, b1, b2.");
  params.addRequiredParam<Real>("wi", "Double well height parameter.");
  params.addRequiredParam<MaterialPropertyName>(
      "gi_name", "Base name for the double well function g_i(eta_i).");
  return params;
}

NestedKKSMultiACBulkF::NestedKKSMultiACBulkF(const InputParameters & parameters)
  : KKSMultiACBulkBase(parameters),
    _c_names(coupledNames("global_cs")),
    _c_map(getParameterJvarMap("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _eta_names(coupledNames("all_etas")),
    _eta_map(getParameterJvarMap("all_etas")),
    _k(-1),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _dcidetaj(_num_c),
    _dcidb(_num_c),
    _wi(getParam<Real>("wi")),
    _gi_name(getParam<MaterialPropertyName>("gi_name")),
    _dgi(getMaterialPropertyDerivative<Real>("gi_name", _etai_name)),
    _d2gi(getMaterialPropertyDerivative<Real>("gi_name", _etai_name, _etai_name)),
    _d2hjdetaidetap(_num_j),
    _dF1dc1(_num_c),
    _dFidarg(_num_j)
{
  for (const auto i : make_range(_num_j))
  {
    // get order parameter names and variable indices
    _eta_names[i] = getVar("all_etas", i)->name();

    // Set _k to the position of the nonlinear variable in the list of etaj's
    if (coupled("all_etas", i) == _var.number())
      _k = i;
  }

  // _dcideta and _dcidb are computed in KKSPhaseConcentrationDerivatives
  for (const auto m : make_range(_num_c))
  {
    _dcidetaj[m].resize(_num_j);
    _dcidb[m].resize(_num_j);

    for (const auto n : make_range(_num_j))
    {
      _dcidetaj[m][n].resize(_num_j);
      _dcidb[m][n].resize(_num_c);

      for (const auto l : make_range(_num_j))
        _dcidetaj[m][n][l] =
            &getMaterialPropertyDerivative<Real>(_ci_names[n + m * _num_j], _eta_names[l]);

      for (const auto l : make_range(_num_c))
        _dcidb[m][n][l] =
            &getMaterialPropertyDerivative<Real>(_ci_names[n + m * _num_j], _c_names[l]);
    }
  }

  // _dF1dc1 is computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_num_c))
    _dF1dc1[m] = &getMaterialPropertyDerivative<Real>(_Fj_names[0], _ci_names[m * _num_j]);

  for (const auto m : make_range(_num_j))
  {
    _d2hjdetaidetap[m].resize(_num_j);

    for (const auto n : make_range(_num_j))
      _d2hjdetaidetap[m][n] =
          &getMaterialPropertyDerivative<Real>(_hj_names[m], _eta_names[_k], _eta_names[n]);
  }

  // _dFidarg is computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_num_j))
  {
    _dFidarg[m].resize(_n_args);

    for (const auto n : make_range(_n_args))
      _dFidarg[m][n] = &getMaterialPropertyDerivative<Real>(_Fj_names[m], m);
  }
}

Real
NestedKKSMultiACBulkF::computeDFDOP(PFFunctionType type)
{
  Real sum = 0.0;

  switch (type)
  {
    case Residual:
      for (const auto m : make_range(_num_j))
        sum += (*_prop_dhjdetai[m])[_qp] * (*_prop_Fj[m])[_qp];

      return sum + _wi * _dgi[_qp];

    case Jacobian:
      // For when this kernel is used in the Lagrange multiplier equation. In that case the
      // Lagrange multiplier is the nonlinear variable
      if (_etai_var != _var.number())
        return 0.0;

      // if eta_i is the nonlinear variable
      for (const auto m : make_range(_num_j))
      {
        Real sum1 = 0.0;

        for (const auto n : make_range(_num_c))
          sum1 += (*_dF1dc1[n])[_qp] * (*_dcidetaj[n][m][_k])[_qp];

        sum +=
            (*_d2hjdetaidetap[m][_k])[_qp] * (*_prop_Fj[m])[_qp] + (*_prop_dhjdetai[m])[_qp] * sum1;
      }

      return _phi[_j][_qp] * (sum + _wi * _d2gi[_qp]);
  }

  mooseError("Invalid type passed in");
}

Real
NestedKKSMultiACBulkF::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class member function Real
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  Real sum = 0.0;

  // if concentrations are the coupled variables
  auto compvar = mapJvarToCvar(jvar, _c_map);
  if (compvar >= 0)
  {
    for (const auto m : make_range(_num_j))
    {
      Real sum1 = 0.0;

      for (const auto n : make_range(_num_c))
        sum1 += (*_dF1dc1[n])[_qp] * (*_dcidb[n][m][compvar])[_qp];

      sum += (*_prop_dhjdetai[m])[_qp] * sum1;
    }

    res += _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

    return res;
  }

  // if order parameters are the coupled variables
  auto etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
  {
    for (const auto m : make_range(_num_j))
    {
      Real sum1 = 0.0;

      for (const auto n : make_range(_num_c))
        sum1 += (*_dF1dc1[n])[_qp] * (*_dcidetaj[n][m][etavar])[_qp];

      sum += (*_d2hjdetaidetap[m][etavar])[_qp] * (*_prop_Fj[m])[_qp] +
             (*_prop_dhjdetai[m])[_qp] * sum1;
    }

    res += _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

    return res;
  }

  // Handle the case when this kernel is used in the Lagrange multiplier equation. In this case
  // the second derivative of the barrier function contributes to the off-diagonal Jacobian
  if (jvar == _etai_var)
  {
    sum += _wi * _d2gi[_qp];

    res += _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  // for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);
  for (const auto m : make_range(_num_j))
    res -= _L[_qp] * (*_prop_dhjdetai[m])[_qp] * (*_dFidarg[m][cvar])[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];

  return res;
}
