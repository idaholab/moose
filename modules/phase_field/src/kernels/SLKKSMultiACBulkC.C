//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SLKKSMultiACBulkC.h"

registerMooseObject("PhaseFieldApp", SLKKSMultiACBulkC);

InputParameters
SLKKSMultiACBulkC::validParams()
{
  auto params = SLKKSMultiPhaseBase::validParams();
  params.addClassDescription("Multi-phase SLKKS model kernel for the bulk Allen-Cahn. "
                             "This includes all terms dependent on chemical potential.");
  params.addRequiredParam<MaterialPropertyName>(
      "F", "Phase free energy function that is a function of 'c'");
  params.addRequiredCoupledVar("c", "Concentration variable F depends on");
  params.addCoupledVar("eta_i",
                       "Order parameter that derivatives are taken with respect to (kernel "
                       "variable is used if this is not specified)");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  return params;
}

SLKKSMultiACBulkC::SLKKSMultiACBulkC(const InputParameters & parameters)
  : SLKKSMultiPhaseBase(parameters),
    _c_name(coupledName("c", 0)),
    _lagrange(isCoupled("eta_i")),
    _etai_name(_lagrange ? coupledName("eta_i", 0) : _var.name()),
    _etai_var(_lagrange ? coupled("eta_i") : _var.number()),
    _cs_names(_ncs),
    _prop_dFdc(getMaterialPropertyDerivative<Real>("F", _c_name)),
    _prop_d2Fdcdcs(_ncs),
    _prop_dhdni(_nh),
    _prop_d2hdnidn(_nh),
    _l_cs(-1),
    _l_etai(-1),
    _mob(getMaterialProperty<Real>("mob_name"))
{
  // Determine position of the selected concentration variable
  for (std::size_t i = 0; i < _ncs; ++i)
  {
    _cs_names[i] = coupledName("cs", i);
    if (coupled("cs", i) == _c_var)
      _l_cs = i;
  }

  // Check to make sure the nonlinear variable is in the cs list
  if (_l_cs < 0)
    paramError("cs", "One of the listed variables must be the 'c' variable");

  // Determine position of the selected concentration variable
  for (std::size_t i = 0; i < _neta; ++i)
    if (coupled("eta", i) == _etai_var)
      _l_etai = i;

  // Check to make sure the nonlinear variable or eta_i (if supplied) is in the eta list
  if (_l_etai < 0)
    paramError("eta_i",
               "Either eta_i or the kernel variable must be one of the listed 'eta' variables");

  // Iterate over all sublattice concentrations
  for (std::size_t i = 0; i < _ncs; ++i)
    // get second partial derivatives w.r.t. c and cs (only if c and cs are in the same phase)
    _prop_d2Fdcdcs[i] = (_phase[i] == _phase[_l_cs])
                            ? &getMaterialPropertyDerivative<Real>("F", _c_name, _cs_names[i])
                            : nullptr;

  // fetch all switching function derivatives
  for (std::size_t i = 0; i < _nh; ++i)
  {
    _prop_dhdni[i] = &getMaterialPropertyDerivativeByName<Real>(_h_names[i], _etai_name);

    _prop_d2hdnidn[i].resize(_neta);
    for (std::size_t j = 0; j < _neta; ++j)
      _prop_d2hdnidn[i][j] =
          &getMaterialPropertyDerivativeByName<Real>(_h_names[i], _etai_name, _eta_names[j]);
  }
}

Real
SLKKSMultiACBulkC::precomputeQpResidual()
{ // sum over phases
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
    sum += (*_prop_dhdni[i])[_qp] * csum;
  }

  return -_mob[_qp] * _prop_dFdc[_qp] / _a_cs[_l_cs] * sum;
}

Real
SLKKSMultiACBulkC::precomputeQpJacobian()
{
  // For when this kernel is used in the Lagrange multiplier equation
  if (_lagrange)
    return 0.0;

  // For when eta_i is the nonlinear variable
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
    sum += (*_prop_d2hdnidn[i][_l_etai])[_qp] * csum;
  }

  return -_mob[_qp] * _phi[_j][_qp] * _prop_dFdc[_qp] / _a_cs[_l_cs] * sum;
}

Real
SLKKSMultiACBulkC::computeQpOffDiagJacobian(unsigned int jvar)
{
  // concentration variables
  auto csvar = mapJvarToCvar(jvar, _cs_map);
  if (csvar >= 0)
  {
    // does F depend on the csvar? if so we need to apply the product rule
    if (_phase[csvar] == _phase[_l_cs])
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
        sum += (*_prop_dhdni[i])[_qp] * csum;
      }

      return -_mob[_qp] *
             ((*_prop_d2Fdcdcs[csvar])[_qp] * sum +
              _prop_dFdc[_qp] * (*_prop_dhdni[_phase[csvar]])[_qp] * _a_cs[csvar]) /
             _a_cs[_l_cs] * _phi[_j][_qp] * _test[_i][_qp];
    }

    return -_mob[_qp] * _prop_dFdc[_qp] / _a_cs[_l_cs] * (*_prop_dhdni[_phase[csvar]])[_qp] *
           _a_cs[csvar] * _phi[_j][_qp] * _test[_i][_qp];
  }

  // order parameters
  auto etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
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
      sum += (*_prop_d2hdnidn[i][etavar])[_qp] * csum;
    }

    return -_mob[_qp] * _prop_dFdc[_qp] / _a_cs[_l_cs] * sum * _phi[_j][_qp] * _test[_i][_qp];
  }

  return 0.0;
}
