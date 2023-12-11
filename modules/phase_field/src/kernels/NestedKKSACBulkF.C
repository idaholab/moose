//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedKKSACBulkF.h"

registerMooseObject("PhaseFieldApp", NestedKKSACBulkF);

InputParameters
NestedKKSACBulkF::validParams()
{
  InputParameters params = KKSACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 1 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms NOT dependent on chemical potential.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "ci_names",
      "Phase concentrations. The order must match Fa, Fb, and global_cs, for example, c1, "
      "c2, b1, b2, etc");
  params.addRequiredParam<MaterialPropertyName>(
      "fb_name", "Free energy function Fb (fa_name is in the KKSACBulkBase).");
  params.addParam<MaterialPropertyName>(
      "g_name", "g", "Base name for the double well function g(eta)");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  return params;
}

NestedKKSACBulkF::NestedKKSACBulkF(const InputParameters & parameters)
  : KKSACBulkBase(parameters),
    _c_map(getParameterJvarMap("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _c_names(coupledNames("global_cs")),
    _ci_names(getParam<std::vector<MaterialPropertyName>>("ci_names")),
    _Fa_name(getParam<MaterialPropertyName>("fa_name")),
    _dFadca(_num_c),
    _Fb_name(getParam<MaterialPropertyName>("fb_name")),
    _dFbdcb(_num_c),
    _prop_dg(getMaterialPropertyDerivative<Real>("g_name", _eta_name)),
    _prop_d2g(getMaterialPropertyDerivative<Real>("g_name", _eta_name, _eta_name)),
    _w(getParam<Real>("w")),
    _prop_Fi(2),
    _dcideta(_num_c * 2),
    _dcidb(_num_c * _num_c * 2),
    _dFadarg(_n_args),
    _dFbdarg(_n_args)
{
  // _prop_Fi is computed in KKSPhaseConcentrationMaterial
  _prop_Fi[0] = &getMaterialPropertyByName<Real>("cp" + _Fa_name);
  _prop_Fi[1] = &getMaterialPropertyByName<Real>("cp" + _Fb_name);

  // _dcideta and _dcid are computed in KKSPhaseConcentrationDerivatives
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

  // _dFadca and _dFbdcb are computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_num_c))
  {
    _dFadca[m] = &getMaterialPropertyDerivative<Real>("cp" + _Fa_name, _ci_names[m * 2]);
    _dFbdcb[m] = &getMaterialPropertyDerivative<Real>("cp" + _Fb_name, _ci_names[m * 2 + 1]);
  }

  // _dFadarg and _dFbdarg are computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_n_args))
  {
    _dFadarg[m] = &getMaterialPropertyDerivative<Real>("cp" + _Fa_name, m);
    _dFbdarg[m] = &getMaterialPropertyDerivative<Real>("cp" + _Fb_name, m);
  }
}

Real
NestedKKSACBulkF::computeDFDOP(PFFunctionType type)
{
  const Real A1 = (*_prop_Fi[0])[_qp] - (*_prop_Fi[1])[_qp];
  switch (type)
  {
    case Residual:
      return -_prop_dh[_qp] * A1 + _w * _prop_dg[_qp];

    case Jacobian:
      Real sum = 0.0;
      for (const auto m : make_range(_num_c))
        sum += (*_dFadca[m])[_qp] * (*_dcideta[m][0])[_qp] -
               (*_dFbdcb[m])[_qp] * (*_dcideta[m][1])[_qp];

      return (-(_prop_d2h[_qp] * A1 + _prop_dh[_qp] * sum) + _w * _prop_d2g[_qp]) * _phi[_j][_qp];
  }
  mooseError("Invalid type passed in");
}

Real
NestedKKSACBulkF::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  // Then add dependence of KKSACBulkF on other variables, and treat c specially using chain rule
  auto compvar = mapJvarToCvar(jvar, _c_map);

  if (compvar >= 0)
  {
    Real sum = 0.0;
    for (const auto m : make_range(_num_c))
      sum += (*_dFadca[m])[_qp] * (*_dcidb[m][0][compvar])[_qp] -
             (*_dFbdcb[m])[_qp] * (*_dcidb[m][1][compvar])[_qp];

    res -= _L[_qp] * _prop_dh[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  // for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  res -= _L[_qp] * _prop_dh[_qp] * ((*_dFadarg[cvar])[_qp] - (*_dFbdarg[cvar])[_qp]) *
         _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
