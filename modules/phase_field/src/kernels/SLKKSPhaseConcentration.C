//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SLKKSPhaseConcentration.h"

registerMooseObject("PhaseFieldApp", SLKKSPhaseConcentration);

InputParameters
SLKKSPhaseConcentration::validParams()
{
  auto params = Kernel::validParams();
  params.addClassDescription(
      "Sublattice KKS model kernel to enforce the decomposition of concentration into "
      "phase and sublattice concentrations The non-linear variable of this kernel is a sublattice "
      "concentration of phase b.");
  params.addRequiredCoupledVar("ca", "Phase a sublattice concentrations");
  params.addRequiredParam<std::vector<Real>>("aa", "Phase a sublattice site fraction");
  params.addRequiredCoupledVar(
      "cb", "Phase b sublattice concentrations (except for the kernel variable)");
  params.addRequiredParam<std::vector<Real>>("ab", "Phase b sublattice site fraction");
  params.addRequiredParam<Real>("a",
                                "Sublattice site fraction for the kernel variable (in phase b)");
  params.addRequiredCoupledVar("c", "Global concentration");
  params.addRequiredCoupledVar("eta", "Phase a/b order parameter");
  params.addParam<MaterialPropertyName>(
      "h_name", "h", "Base name for the switching function h(eta)");
  return params;
}

// Phase interpolation func
SLKKSPhaseConcentration::SLKKSPhaseConcentration(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _nca(coupledComponents("ca")),
    _ca(_nca),
    _a_ca(getParam<std::vector<Real>>("aa")),
    _ca_map(getParameterJvarMap("ca")),
    _ncb(coupledComponents("cb")),
    _cb(_ncb),
    _a_cb(getParam<std::vector<Real>>("ab")),
    _cb_map(getParameterJvarMap("cb")),
    _a_u(getParam<Real>("a")),
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _eta(coupledValue("eta")),
    _eta_var(coupled("eta")),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _prop_dh(getMaterialPropertyDerivative<Real>("h_name", coupledName("eta", 0)))
{
  if (_a_ca.size() != _nca)
    paramError("aa", "Specify one sublattice site fraction per sublattice concentration variable");
  if (_a_cb.size() != _ncb)
    paramError("ab", "Specify one sublattice site fraction per sublattice concentration variable");

  // check and re-normalize sublattice A site fractions
  Real sum = 0.0;
  for (std::size_t i = 0; i < _nca; ++i)
    sum += _a_ca[i];
  if (sum <= 0.0)
    paramError("aa", "The sum of the aa values must be greater than zero");
  for (std::size_t i = 0; i < _nca; ++i)
    _a_ca[i] /= sum;

  // check and re-normalize sublattice B site fractions
  sum = _a_u;
  for (std::size_t i = 0; i < _ncb; ++i)
    sum += _a_cb[i];
  if (sum <= 0.0)
    paramError("ab", "The sum of the ab values and k must be greater than zero");
  for (std::size_t i = 0; i < _ncb; ++i)
    _a_cb[i] /= sum;
  _a_u /= sum;

  // fetch coupled concentrations
  for (std::size_t i = 0; i < _nca; ++i)
    _ca[i] = &coupledValue("ca", i);
  for (std::size_t i = 0; i < _ncb; ++i)
    _cb[i] = &coupledValue("cb", i);
}

Real
SLKKSPhaseConcentration::computeQpResidual()
{
  computeSums();
  return _test[_i][_qp] * ((1.0 - _prop_h[_qp]) * _casum + _prop_h[_qp] * _cbsum - _c[_qp]);
}

Real
SLKKSPhaseConcentration::computeQpJacobian()
{
  return _test[_i][_qp] * _prop_h[_qp] * _phi[_j][_qp] * _a_u;
}

Real
SLKKSPhaseConcentration::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return -_test[_i][_qp] * _phi[_j][_qp];

  if (jvar == _eta_var)
  {
    computeSums();
    return _test[_i][_qp] * (_cbsum - _casum) * _prop_dh[_qp] * _phi[_j][_qp];
  }

  auto cavar = mapJvarToCvar(jvar, _ca_map);
  if (cavar >= 0)
    return _test[_i][_qp] * (1.0 - _prop_h[_qp]) * _phi[_j][_qp] * _a_ca[cavar];

  auto cbvar = mapJvarToCvar(jvar, _cb_map);
  if (cbvar >= 0)
    return _test[_i][_qp] * _prop_h[_qp] * _phi[_j][_qp] * _a_cb[cbvar];

  return 0.0;
}

void
SLKKSPhaseConcentration::computeSums()
{
  _casum = 0.0;
  for (std::size_t i = 0; i < _nca; ++i)
    _casum += (*_ca[i])[_qp] * _a_ca[i];

  _cbsum = _u[_qp] * _a_u;
  for (std::size_t i = 0; i < _ncb; ++i)
    _cbsum += (*_cb[i])[_qp] * _a_cb[i];
}
