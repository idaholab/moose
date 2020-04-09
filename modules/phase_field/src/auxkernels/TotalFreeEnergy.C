//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalFreeEnergy.h"

registerMooseObject("PhaseFieldApp", TotalFreeEnergy);

InputParameters
TotalFreeEnergy::validParams()
{
  InputParameters params = TotalFreeEnergyBase::validParams();
  params.addClassDescription("Total free energy (both the bulk and gradient parts), where the bulk "
                             "free energy has been defined in a material");
  params.addParam<MaterialPropertyName>("f_name", "F", " Base name of the free energy function");
  params.addParam<std::vector<MaterialPropertyName>>("kappa_names",
                                                     std::vector<MaterialPropertyName>(),
                                                     "Vector of kappa names corresponding to "
                                                     "each variable name in interfacial_vars "
                                                     "in the same order.");
  return params;
}

TotalFreeEnergy::TotalFreeEnergy(const InputParameters & parameters)
  : TotalFreeEnergyBase(parameters), _F(getMaterialProperty<Real>("f_name")), _kappas(_nkappas)
{
  // Error check to ensure size of interfacial_vars is the same as kappa_names
  if (_nvars != _nkappas)
    mooseError(
        "Size of interfacial_vars is not equal to the size of kappa_names in TotalFreeEnergy");

  // Assign kappa values
  for (unsigned int i = 0; i < _nkappas; ++i)
    _kappas[i] = &getMaterialPropertyByName<Real>(_kappa_names[i]);
}

Real
TotalFreeEnergy::computeValue()
{
  // Include bulk energy contribution and additional contributions
  Real total_energy = _F[_qp] + _additional_free_energy[_qp];

  // Calculate interfacial energy of each variable
  for (unsigned int i = 0; i < _nvars; ++i)
    total_energy += (*_kappas[i])[_qp] / 2.0 * (*_grad_vars[i])[_qp].norm_sq();

  return total_energy;
}
