//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedKKSMultiACBulkC.h"

registerMooseObject("PhaseFieldApp", NestedKKSMultiACBulkC);

InputParameters
NestedKKSMultiACBulkC::validParams()
{
  InputParameters params = KKSMultiACBulkBase::validParams();
  params.addClassDescription("Multi-phase KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. "
                             "This includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar("global_cs", "Global concentrations, for example, c, b.");
  params.addRequiredCoupledVar("all_etas", "Order parameters.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. They must have the same order as Fj_names and global_cs, for "
      "example, c1, c2, b1, b2.");
  return params;
}

NestedKKSMultiACBulkC::NestedKKSMultiACBulkC(const InputParameters & parameters)
  : KKSMultiACBulkBase(parameters),
    _c_names(coupledNames("global_cs")),
    _c_map(getParameterJvarMap("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _eta_names(coupledNames("all_etas")),
    _eta_map(getParameterJvarMap("all_etas")),
    _k(-1),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _ci_name_matrix(_num_c),
    _prop_ci(_num_c),
    _dcidetaj(_num_c),
    _dcidb(_num_c),
    _prop_d2hjdetaidetap(_num_j),
    _dF1dc1(_num_c),
    _d2F1dc1db1(_num_c),
    _d2F1dc1darg(_n_args)
{
  for (unsigned int i = 0; i < _num_j; ++i)
  {
    // get order parameter names and variable indices
    _eta_names[i] = getVar("all_etas", i)->name();

    // Set _k to the position of the nonlinear variable in the list of etaj's
    if (coupled("all_etas", i) == _var.number())
      _k = i;
  }

  // initialize _ci_name_matrix and _prop_ci for easy reference
  for (unsigned int m = 0; m < _num_c; ++m)
  {
    _ci_name_matrix[m].resize(_num_j);
    _prop_ci[m].resize(_num_j);

    for (const auto n : make_range(_num_j))
    {
      _ci_name_matrix[m][n] = _ci_names[m * _num_j + n];
      _prop_ci[m][n] = &getMaterialPropertyByName<Real>(_ci_name_matrix[m][n]);
    }
  }

  // _dcidetaj and _dcidb are computed in KKSPhaseConcentrationMultiPhaseDerivatives
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

  for (const auto m : make_range(_num_j))
  {
    _prop_d2hjdetaidetap[m].resize(_num_j);

    for (const auto n : make_range(_num_j))
      _prop_d2hjdetaidetap[m][n] =
          &getMaterialPropertyDerivative<Real>(_hj_names[m], _eta_names[_k], _eta_names[n]);
  }

  // _dF1dc1 and _d2F1dc1db1 are computed in KKSPhaseConcentrationMultiPhaseMaterial
  for (const auto m : make_range(_num_c))
  {
    _dF1dc1[m] = &getMaterialPropertyDerivative<Real>(_Fj_names[0], _ci_names[m * _num_j]);
    _d2F1dc1db1[m].resize(_num_c);

    for (const auto n : make_range(_num_c))
      _d2F1dc1db1[m][n] = &getMaterialPropertyDerivative<Real>(
          _Fj_names[0], _ci_name_matrix[m][0], _ci_name_matrix[n][0]);
  }

  // _d2F1dc1darg are computed in KKSPhaseConcentrationMultiPhaseMaterial
  for (const auto m : make_range(_num_c))
  {
    _d2F1dc1darg[m].resize(_n_args);

    for (const auto n : make_range(_n_args))
      _d2F1dc1darg[m][n] =
          &getMaterialPropertyDerivative<Real>(_Fj_names[0], _ci_name_matrix[m][0], n);
  }
}

Real
NestedKKSMultiACBulkC::computeDFDOP(PFFunctionType type)
{
  Real sum = 0.0;

  switch (type)
  {
    case Residual:

      for (const auto m : make_range(_num_c))
      {
        Real sum1 = 0.0;

        for (const auto n : make_range(_num_j))
          sum1 += (*_prop_dhjdetai[n])[_qp] * (*_prop_ci[m][n])[_qp];

        sum += (*_dF1dc1[m])[_qp] * sum1;
      }

      return -sum;

    case Jacobian:
      /** For when this kernel is used in the Lagrange multiplier equation
          In that case the Lagrange multiplier is the nonlinear variable */
      if (_etai_var != _var.number())
        return 0.0;

      for (const auto m : make_range(_num_c))
      {
        Real sum1 = 0.0;
        Real sum2 = 0.0;
        Real sum3 = 0.0;

        for (const auto n : make_range(_num_c))
          sum1 += (*_d2F1dc1db1[m][n])[_qp] * (*_dcidetaj[n][0][_k])[_qp];

        for (const auto l : make_range(_num_j))
        {
          sum2 += (*_prop_dhjdetai[l])[_qp] * (*_prop_ci[m][l])[_qp];

          sum3 += (*_prop_d2hjdetaidetap[l][_k])[_qp] * (*_prop_ci[m][l])[_qp] +
                  (*_prop_dhjdetai[l])[_qp] * (*_dcidetaj[m][l][_k])[_qp];
        }

        sum += sum1 * sum2 + (*_dF1dc1[m])[_qp] * sum3;
      }

      return -sum * _phi[_j][_qp];
  }

  mooseError("Invalid type passed in");
}

Real
NestedKKSMultiACBulkC::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class member function Real
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  Real sum = 0.0;

  // Then add dependence of KKSACBulkF on other variables if other cs are the coupled variables
  auto compvar = mapJvarToCvar(jvar, _c_map);
  if (compvar >= 0)
  {
    for (const auto m : make_range(_num_c))
    {
      Real sum1 = 0.0;
      Real sum2 = 0.0;
      Real sum3 = 0.0;

      for (const auto n : make_range(_num_c))
        sum1 += (*_d2F1dc1db1[m][n])[_qp] * (*_dcidb[n][0][compvar])[_qp];

      for (const auto l : make_range(_num_j))
      {
        sum2 += (*_prop_dhjdetai[l])[_qp] * (*_prop_ci[m][l])[_qp];
        sum3 += (*_prop_dhjdetai[l])[_qp] * (*_dcidb[m][l][compvar])[_qp];
      }

      sum += sum1 * sum2 + (*_dF1dc1[m])[_qp] * sum3;
    }

    res += -_L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

    return res;
  }

  // if order parameters are the coupled variables
  auto etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
  {
    for (const auto m : make_range(_num_c))
    {
      Real sum1 = 0.0;
      Real sum2 = 0.0;
      Real sum3 = 0.0;

      for (const auto n : make_range(_num_c))
        sum1 += (*_d2F1dc1db1[m][n])[_qp] * (*_dcidetaj[n][0][etavar])[_qp];

      for (const auto l : make_range(_num_j))
      {
        sum2 += (*_prop_dhjdetai[l])[_qp] * (*_prop_ci[m][l])[_qp];

        sum3 += (*_prop_d2hjdetaidetap[l][etavar])[_qp] * (*_prop_ci[m][l])[_qp] +
                (*_prop_dhjdetai[l])[_qp] * (*_dcidetaj[m][l][etavar])[_qp];
      }

      sum += sum1 * sum2 + (*_dF1dc1[m])[_qp] * sum3;
    }

    res += -_L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  // for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);
  for (unsigned int m = 0; m < _num_c; ++m)
  {
    Real sum1 = 0.0;

    for (const auto n : make_range(_num_j))
      sum1 += (*_d2F1dc1darg[m][cvar])[_qp] * (*_prop_dhjdetai[n])[_qp] * (*_prop_ci[m][n])[_qp];

    res += -_L[_qp] * sum1 * _phi[_j][_qp] * _test[_i][_qp];
  }

  return res;
}
