//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SLKKSMultiPhaseBase.h"

InputParameters
SLKKSMultiPhaseBase::validParams()
{
  auto params = Kernel::validParams();
  params.addRequiredCoupledVar("cs", "Array of sublattice concentrations phase for all phases");
  params.addCoupledVar("eta", "Order parameters for all phases");
  params.addRequiredParam<std::vector<unsigned int>>("ns", "Number of sublattices in each phase");
  params.addRequiredParam<std::vector<Real>>("as", "Sublattice site fractions n all phases");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "h_names", "Switching Function Materials for all phases");
  return params;
}

// Phase interpolation func
SLKKSMultiPhaseBase::SLKKSMultiPhaseBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<KernelValue>>(parameters),
    _ncs(coupledComponents("cs")),
    _cs(_ncs),
    _cs_map(getParameterJvarMap("cs")),
    _ns(getParam<std::vector<unsigned int>>("ns")),
    _neta(coupledComponents("eta")),
    _eta_names(_neta),
    _eta_map(getParameterJvarMap("eta")),
    _a_cs(getParam<std::vector<Real>>("as")),
    _h_names(getParam<std::vector<MaterialPropertyName>>("h_names")),
    _nh(_h_names.size()),
    _phase(_ncs),
    _c(coupledValue("c")),
    _c_var(coupled("c"))
{
  // consistent number of phases?
  if (_ns.size() != _nh)
    paramError("ns",
               "Need to pass in as many switching functions (parameter `h_names`) as size of "
               "number of sublattices in each phase (parameter `ns`)");

  // sum up ns numbers
  unsigned int nssum = 0;
  for (auto i : _ns)
  {
    if (i == 0)
      paramError("ns", "All sublattice counts must be larger than zero");
    nssum += i;
  }

  if (nssum != _ncs)
    paramError("ns", "Numbers need to sum up to the number of passed in cs variables");
  if (_ncs == 0)
    paramError("cs", "Need to supply at least 1 phase sublattice concentration");

  // get order parameter names
  for (std::size_t i = 0; i < _neta; ++i)
    _eta_names[i] = coupledName("eta", i);

  // Load concentration variables into the arrays
  for (std::size_t i = 0; i < _ncs; ++i)
    _cs[i] = &coupledValue("cs", i);

  // normalize sublattice counts within each phase
  std::size_t k = 0;
  for (std::size_t i = 0; i < _nh; ++i)
  {
    // sum site counts
    Real sum = _a_cs[k];
    for (unsigned int j = 1; j < _ns[i]; ++j)
      sum += _a_cs[j + k];
    if (sum <= 0)
      paramError("as", "Sum of sublattice site counts in each phase must be larger than zero");

    // normalize
    for (unsigned int j = 0; j < _ns[i]; ++j)
    {
      _a_cs[j + k] /= sum;

      // remember phase index
      _phase[j + k] = i;
    }

    // next phase
    k += _ns[i];
  }

  // consistency checks
  if (k != _ncs)
    mooseError("Internal error");
}
