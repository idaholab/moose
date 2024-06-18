//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedKKSSplitCHCRes.h"

registerMooseObject("PhaseFieldApp", NestedKKSSplitCHCRes);

InputParameters
NestedKKSSplitCHCRes::validParams()
{
  InputParameters params = JvarMapKernelInterface<Kernel>::validParams();
  params.addClassDescription(
      "KKS model kernel for the split Bulk Cahn-Hilliard term. This kernel operates on the "
      "physical concentration 'c' as the non-linear variable.");
  params.addCoupledVar("all_etas", "Phase parameters for all phases.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc.");
  params.addCoupledVar("w", "Chemical potential non-linear helper variable for the split solve.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "ca_names",
      "Phase concentrations in the frist phase of all_etas. The order must match global_cs, for "
      "example, c1, b1, etc.");
  params.addParam<MaterialPropertyName>("fa_name", "Free energy of the first phase in all_etas.");
  return params;
}

NestedKKSSplitCHCRes::NestedKKSSplitCHCRes(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _eta_names(coupledNames("all_etas")),
    _num_j(_eta_names.size()),
    _eta_map(getParameterJvarMap("all_etas")),
    _c_names(coupledNames("global_cs")),
    _num_c(coupledComponents("global_cs")),
    _c_map(getParameterJvarMap("global_cs")),
    _o(-1),
    _w_var(coupled("w")),
    _w(coupledValue("w")),
    _ca_names(getParam<std::vector<MaterialPropertyName>>("ca_names")),
    _Fa_name(getParam<MaterialPropertyName>("fa_name")),
    _dFadca(_num_c),
    _d2Fadcadba(_num_c),
    _dcadb(_num_c),
    _dcadetaj(_num_c),
    _d2Fadcadarg(_n_args)

{
  for (const auto i : make_range(_num_c))
  {
    // Set _o to the position of the nonlinear variable in the list of global_cs
    if (coupled("global_cs", i) == _var.number())
      _o = i;
  }

  // _dcideta and _dcidb are computed in KKSPhaseConcentrationDerivatives
  for (const auto m : make_range(_num_c))
  {
    _dcadetaj[m].resize(_num_j);
    for (const auto n : make_range(_num_j))
      _dcadetaj[m][n] = &getMaterialPropertyDerivative<Real>(_ca_names[m], _eta_names[n]);

    _dcadb[m].resize(_num_c);
    for (const auto n : make_range(_num_c))
      _dcadb[m][n] = &getMaterialPropertyDerivative<Real>(_ca_names[m], _c_names[n]);
  }

  // _dFaca and _d2Fadcadba are computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_num_c))
  {
    _dFadca[m] = &getMaterialPropertyDerivative<Real>(_Fa_name, _ca_names[m]);
    _d2Fadcadba[m] = &getMaterialPropertyDerivative<Real>(_Fa_name, _ca_names[_o], _ca_names[m]);
  }

  // _d2Fadcadarg is computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_n_args))
    _d2Fadcadarg[m] = &getMaterialPropertyDerivative<Real>(_Fa_name, _ca_names[_o], m);
}

Real
NestedKKSSplitCHCRes::computeQpResidual()
{
  return ((*_dFadca[_o])[_qp] - _w[_qp]) * _test[_i][_qp];
}

Real
NestedKKSSplitCHCRes::computeQpJacobian()
{
  Real sum = 0.0;

  for (const auto m : make_range(_num_c))
    sum += (*_d2Fadcadba[m])[_qp] * (*_dcadb[m][_o])[_qp];

  return sum * _phi[_j][_qp] * _test[_i][_qp];
}

Real
NestedKKSSplitCHCRes::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real sum = 0.0;

  // treat w variable explicitly
  if (jvar == _w_var)
    return -_phi[_j][_qp] * _test[_i][_qp];

  // if b is the coupled variable
  auto compvar = mapJvarToCvar(jvar, _c_map);
  if (compvar >= 0)
  {
    for (const auto m : make_range(_num_c))
      sum += (*_d2Fadcadba[m])[_qp] * (*_dcadb[m][compvar])[_qp];

    return sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  // if order parameters are the coupled variables
  auto etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
  {
    for (const auto m : make_range(_num_c))
      sum += (*_d2Fadcadba[m])[_qp] * (*_dcadetaj[m][etavar])[_qp];

    return sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  // for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);
  return (*_d2Fadcadarg[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
