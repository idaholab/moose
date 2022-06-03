//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SLKKSMultiPhaseConcentration.h"

registerMooseObject("PhaseFieldApp", SLKKSMultiPhaseConcentration);

InputParameters
SLKKSMultiPhaseConcentration::validParams()
{
  auto params = SLKKSMultiPhaseBase::validParams();
  params.addClassDescription(
      "SLKKS multi-phase model kernel to enforce $c_i = \\sum_j h_j\\sum_k a_{jk} c_{ijk}$. "
      "The non-linear variable of this kernel is a phase's sublattice concentration");
  params.addRequiredCoupledVar("c", "Physical concentration");
  return params;
}

// Phase interpolation func
SLKKSMultiPhaseConcentration::SLKKSMultiPhaseConcentration(const InputParameters & parameters)
  : SLKKSMultiPhaseBase(parameters), _l(-1), _prop_h(_nh), _prop_dhdeta(_nh)
{
  // Fetch switching functions and their derivatives
  for (std::size_t i = 0; i < _nh; ++i)
  {
    _prop_h[i] = &getMaterialPropertyByName<Real>(_h_names[i]);
    _prop_dhdeta[i].resize(_neta);

    // Get derivatives of switching functions w.r.t. order parameters
    for (std::size_t j = 0; j < _neta; ++j)
      _prop_dhdeta[i][j] = &getMaterialPropertyDerivativeByName<Real>(_h_names[i], _eta_names[j]);
  }

  // Determine position of the nonlinear variable
  for (std::size_t i = 0; i < _ncs; ++i)
    if (coupled("cs", i) == _var.number())
      _l = i;

  // Check to make sure the nonlinear variable is in the cs list
  if (_l < 0)
    paramError("cs", "One of the listed variables must be the kernel variable");
}

Real
SLKKSMultiPhaseConcentration::precomputeQpResidual()
{
  // sum over phases
  std::size_t k = 0;
  Real sum = 0.0;
  for (std::size_t i = 0; i < _nh; ++i)
  {
    // sum sublattice concentrations
    Real csum = 0.0;
    for (unsigned int j = 0; j < _ns[i]; ++j)
    {
      csum += (*_cs[k])[_qp] * _a_cs[k];
      k++;
    }
    sum += (*_prop_h[i])[_qp] * csum;
  }
  return sum - _c[_qp];
}

Real
SLKKSMultiPhaseConcentration::precomputeQpJacobian()
{
  return (*_prop_h[_phase[_l]])[_qp] * _phi[_j][_qp] * _a_cs[_l];
}

Real
SLKKSMultiPhaseConcentration::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return -_test[_i][_qp] * _phi[_j][_qp];

  auto csvar = mapJvarToCvar(jvar, _cs_map);
  if (csvar >= 0)
    return _test[_i][_qp] * (*_prop_h[_phase[csvar]])[_qp] * _phi[_j][_qp] * _a_cs[csvar];

  auto etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
  {
    Real sum = 0.0;
    for (unsigned int i = 0; i < _ncs; ++i)
      sum += (*_prop_dhdeta[_phase[i]][etavar])[_qp] * (*_cs[i])[_qp] * _a_cs[i];

    return _test[_i][_qp] * sum * _phi[_j][_qp];
  }

  return 0.0;
}
