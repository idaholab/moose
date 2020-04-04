//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrossTermGradientFreeEnergy.h"

registerMooseObject("PhaseFieldApp", CrossTermGradientFreeEnergy);

InputParameters
CrossTermGradientFreeEnergy::validParams()
{
  InputParameters params = TotalFreeEnergyBase::validParams();
  params.addClassDescription("Free energy contribution from the cross terms in ACMultiInterface");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "kappa_names",
      "Matrix of kappa names with rows and columns corresponding to each variable "
      "name in interfacial_vars in the same order (should be symmetric).");
  return params;
}

CrossTermGradientFreeEnergy::CrossTermGradientFreeEnergy(const InputParameters & parameters)
  : TotalFreeEnergyBase(parameters), _kappas(_nvars)
{
  // Error check to ensure size of interfacial_vars is the same as kappa_names
  if (_nvars * _nvars != _nkappas)
    paramError("kappa_names",
               "Size of interfacial_vars squared is not equal to the size of kappa_names in "
               "CrossTermGradientFreeEnergy");

  // Assign kappa values
  for (unsigned int i = 0; i < _nvars; ++i)
  {
    _kappas[i].resize(_nvars);

    for (unsigned int j = 0; j < _nvars; ++j)
      _kappas[i][j] = &getMaterialPropertyByName<Real>(_kappa_names[i * _nvars + j]);
  }
}

Real
CrossTermGradientFreeEnergy::computeValue()
{
  // This kernel does _not_ include the bulk energy contribution.
  // It is to be used as an additional free energy component in TotalFreeEnergy.
  // additional_free_energy can be used to daisy chain contributions!
  Real total_energy = _additional_free_energy[_qp];

  // Calculate interfacial energy of each variable combination
  for (unsigned int i = 0; i < _nvars; ++i)
    for (unsigned int j = 0; j < i; ++j)
    {
      const RealGradient cross =
          (*_vars[i])[_qp] * (*_grad_vars[j])[_qp] + (*_vars[j])[_qp] * (*_grad_vars[i])[_qp];
      total_energy += (*_kappas[i][j])[_qp] / 2.0 * cross * cross;
    }
  return total_energy;
}
