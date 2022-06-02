//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SLKKSSum.h"
#include "MathUtils.h"

registerMooseObject("PhaseFieldApp", SLKKSSum);

InputParameters
SLKKSSum::validParams()
{
  auto params = Kernel::validParams();
  params.addClassDescription(
      "Enforce the sum of sublattice concentrations to a given phase concentration.");
  params.addRequiredCoupledVar("cs", "other sublattice concentrations in the same phase");
  params.addRequiredParam<Real>("a", "Sublattice site fraction for the kernel variable");
  params.addRequiredParam<std::vector<Real>>("as", "Phase a sublattice site fractions");
  params.addRequiredCoupledVar("sum", "prescribed sum");
  return params;
}

SLKKSSum::SLKKSSum(const InputParameters & parameters)
  : JvarMapKernelInterface<Kernel>(parameters),
    _ncs(coupledComponents("cs")),
    _cs(_ncs),
    _a_cs(getParam<std::vector<Real>>("as")),
    _cs_map(getParameterJvarMap("cs")),
    _a_u(getParam<Real>("a")),
    _target(coupledValue("sum"))
{
  if (_a_cs.size() != _ncs)
    paramError("as", "Specify one sublattice site fraction per sublattice concentration variable");

  // check and re-normalize sublattice B site fractions
  Real sum = _a_u;
  for (std::size_t i = 0; i < _ncs; ++i)
    sum += _a_cs[i];
  if (sum <= 0.0)
    paramError("as", "The sum of the 'as' values and 'a' must be greater than zero");
  for (std::size_t i = 0; i < _ncs; ++i)
    _a_cs[i] /= sum;
  _a_u /= sum;

  // fetch coupled concentrations
  for (std::size_t i = 0; i < _ncs; ++i)
    _cs[i] = &coupledValue("cs", i);
}

Real
SLKKSSum::computeQpResidual()
{
  Real csum = _u[_qp] * _a_u;
  for (std::size_t i = 0; i < _ncs; ++i)
    csum += (*_cs[i])[_qp] * _a_cs[i];

  return _test[_i][_qp] * (csum - _target[_qp]);
}

Real
SLKKSSum::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * _a_u;
}

Real
SLKKSSum::computeQpOffDiagJacobian(unsigned int jvar)
{
  auto csvar = mapJvarToCvar(jvar, _cs_map);
  if (csvar >= 0)
    return _test[_i][_qp] * _phi[_j][_qp] * _a_cs[csvar];

  return 0.0;
}
