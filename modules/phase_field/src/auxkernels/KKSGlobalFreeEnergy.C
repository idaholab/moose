//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSGlobalFreeEnergy.h"

registerMooseObject("PhaseFieldApp", KKSGlobalFreeEnergy);

InputParameters
KKSGlobalFreeEnergy::validParams()
{
  InputParameters params = TotalFreeEnergyBase::validParams();
  params.addClassDescription(
      "Total free energy in KKS system, including chemical, barrier and gradient terms");
  params.addRequiredParam<MaterialPropertyName>("fa_name",
                                                "Base name of the free energy function "
                                                "F (f_name in the corresponding "
                                                "derivative function material)");
  params.addRequiredParam<MaterialPropertyName>("fb_name",
                                                "Base name of the free energy function "
                                                "F (f_name in the corresponding "
                                                "derivative function material)");
  params.addParam<MaterialPropertyName>(
      "h_name", "h", "Base name for the switching function h(eta)");
  params.addParam<MaterialPropertyName>(
      "g_name", "g", "Base name for the double well function g(eta)");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  params.addParam<std::vector<MaterialPropertyName>>("kappa_names",
                                                     std::vector<MaterialPropertyName>(),
                                                     "Vector of kappa names corresponding to "
                                                     "each variable name in interfacial_vars "
                                                     "in the same order. For basic KKS, there "
                                                     "is 1 kappa, 1 interfacial_var.");
  return params;
}

KKSGlobalFreeEnergy::KKSGlobalFreeEnergy(const InputParameters & parameters)
  : TotalFreeEnergyBase(parameters),
    _prop_fa(getMaterialProperty<Real>("fa_name")),
    _prop_fb(getMaterialProperty<Real>("fb_name")),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _prop_g(getMaterialProperty<Real>("g_name")),
    _w(getParam<Real>("w")),
    _kappas(_nkappas)
{
  // Error check to ensure size of interfacial_vars is the same as kappa_names
  if (_nvars != _nkappas)
    mooseError(
        "Size of interfacial_vars is not equal to the size of kappa_names in KKSGlobalFreeEnergy");

  // Assign kappa values
  for (unsigned int i = 0; i < _nkappas; ++i)
    _kappas[i] = &getMaterialPropertyByName<Real>(_kappa_names[i]);
}

Real
KKSGlobalFreeEnergy::computeValue()
{
  const Real h = _prop_h[_qp];

  // Include bulk energy and additional contributions
  Real total_energy = _prop_fa[_qp] * (1.0 - h) + _prop_fb[_qp] * h + _w * _prop_g[_qp] +
                      _additional_free_energy[_qp];

  // Calculate interfacial energy of each variable
  for (unsigned int i = 0; i < _nvars; ++i)
    total_energy += (*_kappas[i])[_qp] / 2.0 * (*_grad_vars[i])[_qp].norm_sq();

  return total_energy;
}
