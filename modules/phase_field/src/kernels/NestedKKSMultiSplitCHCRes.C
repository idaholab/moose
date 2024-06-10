//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedKKSMultiSplitCHCRes.h"

registerMooseObject("PhaseFieldApp", NestedKKSMultiSplitCHCRes);

InputParameters
NestedKKSMultiSplitCHCRes::validParams()
{
  InputParameters params = JvarMapKernelInterface<Kernel>::validParams();
  params.addClassDescription(
      "KKS model kernel for the split Bulk Cahn-Hilliard term. This kernel operates on the "
      "physical concentration 'c' as the non-linear variable.");
  params.addCoupledVar("all_etas", "Phase parameters for all phases.");
  params.addRequiredCoupledVar("global_cs", "The interpolated concentrations c, b, etc.");
  params.addCoupledVar("w", "Chemical potential non-linear helper variable for the split solve.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "c1_names",
      "Phase concentrations in the frist phase of all_etas. The order must match global_cs, for "
      "example, c1, b1, etc.");
  params.addParam<MaterialPropertyName>("F1_name", "Free energy of the first phase in all_etas.");
  return params;
}

NestedKKSMultiSplitCHCRes::NestedKKSMultiSplitCHCRes(const InputParameters & parameters)
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
    _c1_names(getParam<std::vector<MaterialPropertyName>>("c1_names")),
    _F1_name(getParam<MaterialPropertyName>("F1_name")),
    _dF1dc1(_num_c),
    _d2F1dc1db1(_num_c),
    _dc1db(_num_c),
    _dc1detaj(_num_c),
    _d2F1dc1darg(_n_args)

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
    _dc1detaj[m].resize(_num_j);
    for (const auto n : make_range(_num_j))
      _dc1detaj[m][n] = &getMaterialPropertyDerivative<Real>(_c1_names[m], _eta_names[n]);

    _dc1db[m].resize(_num_c);
    for (const auto n : make_range(_num_c))
      _dc1db[m][n] = &getMaterialPropertyDerivative<Real>(_c1_names[m], _c_names[n]);
  }

  // _dF1dc1 and _d2F1dc1db1 are computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_num_c))
  {
    _dF1dc1[m] = &getMaterialPropertyDerivative<Real>(_F1_name, _c1_names[m]);
    _d2F1dc1db1[m] = &getMaterialPropertyDerivative<Real>(_F1_name, _c1_names[_o], _c1_names[m]);
  }

  // _d2F1dc1darg is computed in KKSPhaseConcentrationMaterial
  for (const auto m : make_range(_n_args))
    _d2F1dc1darg[m] = &getMaterialPropertyDerivative<Real>(_F1_name, _c1_names[_o], m);
}

Real
NestedKKSMultiSplitCHCRes::computeQpResidual()
{
  return ((*_dF1dc1[_o])[_qp] - _w[_qp]) * _test[_i][_qp];
}

Real
NestedKKSMultiSplitCHCRes::computeQpJacobian()
{
  Real sum = 0.0;

  for (const auto m : make_range(_num_c))
    sum += (*_d2F1dc1db1[m])[_qp] * (*_dc1db[m][_o])[_qp];

  return sum * _phi[_j][_qp] * _test[_i][_qp];
}

Real
NestedKKSMultiSplitCHCRes::computeQpOffDiagJacobian(unsigned int jvar)
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
      sum += (*_d2F1dc1db1[m])[_qp] * (*_dc1db[m][compvar])[_qp];

    return sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  // if order parameters are the coupled variables
  auto etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
  {
    for (const auto m : make_range(_num_c))
      sum += (*_d2F1dc1db1[m])[_qp] * (*_dc1detaj[m][etavar])[_qp];

    return sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  // for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return (*_d2F1dc1darg[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
