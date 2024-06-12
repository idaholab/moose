//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedKKSACBulkC.h"

registerMooseObject("PhaseFieldApp", NestedKKSACBulkC);

InputParameters
NestedKKSACBulkC::validParams()
{
  InputParameters params = KKSACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. The order must match Fa, Fb, and global_cs, for example, c1, "
      "c2, b1, b2, etc");
  return params;
}

NestedKKSACBulkC::NestedKKSACBulkC(const InputParameters & parameters)
  : KKSACBulkBase(parameters),
    _c_names(coupledNames("global_cs")),
    _c_map(getParameterJvarMap("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _prop_ci(_num_c),
    _dcideta(_num_c * 2),
    _dcidb(_num_c * 2),
    _Fa_name(getParam<MaterialPropertyName>("fa_name")),
    _dFadca(_num_c),
    _d2Fadcadba(_num_c),
    _d2Fadcadarg(_n_args)
{
  // Declare _prop_ci to be a matrix for easy reference. In _prop_ci[m][n], m is species index, n
  // is the phase index.
  for (const auto m : make_range(_num_c))
  {
    _prop_ci[m].resize(2);
    for (const auto n : make_range(2))
      _prop_ci[m][n] = &getMaterialPropertyByName<Real>(_ci_names[m * 2 + n]);
  }

  // _dcideta and _dcidb are computed in KKSPhaseConcentrationDerivatives
  for (const auto m : make_range(_num_c))
  {
    _dcideta[m].resize(2);
    _dcidb[m].resize(2);
    for (const auto n : make_range(2))
    {
      _dcideta[m][n] = &getMaterialPropertyDerivative<Real>(_ci_names[m * 2 + n], _var.name());
      _dcidb[m][n].resize(_num_c);
      for (const auto l : make_range(_num_c))
        _dcidb[m][n][l] = &getMaterialPropertyDerivative<Real>(_ci_names[m * 2 + n], _c_names[l]);
    }
  }

  // _dFadca and _d2Fadcadba are computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_num_c))
  {
    _dFadca[m] = &getMaterialPropertyDerivative<Real>("cp" + _Fa_name, _ci_names[m * 2]);
    _d2Fadcadba[m].resize(_num_c);
    for (const auto n : make_range(_num_c))
      _d2Fadcadba[m][n] =
          &getMaterialPropertyDerivative<Real>("cp" + _Fa_name, _ci_names[m * 2], _ci_names[n * 2]);
  }

  // _d2Fadcadarg are computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_num_c))
  {
    _d2Fadcadarg[m].resize(_n_args);
    for (const auto n : make_range(_n_args))
      _d2Fadcadarg[m][n] =
          &getMaterialPropertyDerivative<Real>("cp" + _Fa_name, _ci_names[m * 2], n);
  }
}

Real
NestedKKSACBulkC::computeDFDOP(PFFunctionType type)
{
  Real sum = 0.0;
  switch (type)
  {
    case Residual:
      for (unsigned int m = 0; m < _num_c; ++m)
        sum += (*_dFadca[m])[_qp] * ((*_prop_ci[m][0])[_qp] - (*_prop_ci[m][1])[_qp]);

      return _prop_dh[_qp] * sum;

    case Jacobian:
      Real sum1 = 0.0;
      for (unsigned int m = 0; m < _num_c; ++m)
        sum1 += (*_dFadca[m])[_qp] * ((*_prop_ci[m][0])[_qp] - (*_prop_ci[m][1])[_qp]);

      Real sum2 = 0.0;
      for (unsigned int m = 0; m < _num_c; ++m)
      {
        Real sum3 = 0.0;
        for (unsigned int n = 0; n < _num_c; ++n)
          sum3 += (*_d2Fadcadba[m][n])[_qp] * (*_dcideta[n][0])[_qp];

        sum2 += sum3 * ((*_prop_ci[m][0])[_qp] - (*_prop_ci[m][1])[_qp]) +
                (*_dFadca[m])[_qp] * ((*_dcideta[m][0])[_qp] - (*_dcideta[m][1])[_qp]);
      }

      return (_prop_d2h[_qp] * sum1 + _prop_dh[_qp] * sum2) * _phi[_j][_qp];
  }

  mooseError("Invalid type passed in");
}

Real
NestedKKSACBulkC::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  // Then add dependence of KKSACBulkF on other variables.
  // Treat c specially using chain rule.
  auto compvar = mapJvarToCvar(jvar, _c_map);
  if (compvar >= 0)
  {
    Real sum1 = 0.0;
    for (unsigned int m = 0; m < _num_c; ++m)
    {
      Real sum2 = 0.0;
      for (unsigned int n = 0; n < _num_c; ++n)
        sum2 += (*_d2Fadcadba[m][n])[_qp] * (*_dcidb[n][0][compvar])[_qp];

      sum1 += sum2 * ((*_prop_ci[m][0])[_qp] - (*_prop_ci[m][1])[_qp]) +
              (*_dFadca[m])[_qp] * ((*_dcidb[m][0][compvar])[_qp] - (*_dcidb[m][1][compvar])[_qp]);
    }

    res += _L[_qp] * _prop_dh[_qp] * sum1 * _phi[_j][_qp] * _test[_i][_qp];
  }

  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);
  for (unsigned int n = 0; n < _num_c; ++n)
    res += _L[_qp] * _prop_dh[_qp] * (*_d2Fadcadarg[n][cvar])[_qp] *
           ((*_prop_ci[n][0])[_qp] - (*_prop_ci[n][1])[_qp]) * _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
