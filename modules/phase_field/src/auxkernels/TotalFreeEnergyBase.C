//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalFreeEnergyBase.h"

InputParameters
TotalFreeEnergyBase::validParams()
{
  InputParameters params = AuxKernel::validParams();
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
    _vars(coupledValues("interfacial_vars")),
    _grad_vars(coupledGradients("interfacial_vars")),
    _kappa_names(getParam<std::vector<MaterialPropertyName>>("kappa_names")),
    _nkappas(_kappa_names.size()),
    _additional_free_energy(coupledValue("additional_free_energy"))
{
}
