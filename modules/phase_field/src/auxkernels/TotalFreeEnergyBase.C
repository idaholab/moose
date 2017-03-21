/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TotalFreeEnergyBase.h"

template <>
InputParameters
validParams<TotalFreeEnergyBase>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addCoupledVar("interfacial_vars", "Variable names that contribute to interfacial energy");
  params.addCoupledVar(
      "additional_free_energy",
      0.0,
      "Coupled variable holding additional free energy contributions to be summed up");
  return params;
}

TotalFreeEnergyBase::TotalFreeEnergyBase(const InputParameters & parameters)
  : AuxKernel(parameters),
    _nvars(coupledComponents("interfacial_vars")),
    _vars(_nvars),
    _grad_vars(_nvars),
    _kappa_names(getParam<std::vector<MaterialPropertyName>>("kappa_names")),
    _nkappas(_kappa_names.size()),
    _additional_free_energy(coupledValue("additional_free_energy"))
{
  // Fetch coupled variables and their gradients
  for (unsigned int i = 0; i < _nvars; ++i)
  {
    _vars[i] = &coupledValue("interfacial_vars", i);
    _grad_vars[i] = &coupledGradient("interfacial_vars", i);
  }
}
