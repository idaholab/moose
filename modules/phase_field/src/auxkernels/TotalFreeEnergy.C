/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TotalFreeEnergy.h"

template <>
InputParameters
validParams<TotalFreeEnergy>()
{
  InputParameters params = validParams<TotalFreeEnergyBase>();
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
