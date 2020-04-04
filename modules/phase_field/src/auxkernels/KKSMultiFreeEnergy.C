//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSMultiFreeEnergy.h"

registerMooseObject("PhaseFieldApp", KKSMultiFreeEnergy);

InputParameters
KKSMultiFreeEnergy::validParams()
{
  InputParameters params = TotalFreeEnergyBase::validParams();
  params.addClassDescription("Total free energy in multi-phase KKS system, including chemical, "
                             "barrier and gradient terms");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Fj_names",
      "List of free energies for each phase. Place in same order as hj_names and gj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names",
      "Switching Function Materials that provide h. Place in same order as Fj_names and gj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "gj_names",
      "Barrier Function Materials that provide g. Place in same order as Fj_names and hj_names!");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  params.addParam<std::vector<MaterialPropertyName>>("kappa_names",
                                                     std::vector<MaterialPropertyName>(),
                                                     "Vector of kappa names corresponding to "
                                                     "each variable name in interfacial_vars "
                                                     "in the same order.");
  return params;
}

KKSMultiFreeEnergy::KKSMultiFreeEnergy(const InputParameters & parameters)
  : TotalFreeEnergyBase(parameters),
    _Fj_names(getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_num_j),
    _gj_names(getParam<std::vector<MaterialPropertyName>>("gj_names")),
    _prop_gj(_num_j),
    _w(getParam<Real>("w")),
    _kappas(_nkappas)
{
  // Check that same number of Fj, hj, and gj are passed in
  if (_hj_names.size() != _num_j)
    mooseError("Size of hj_names is not equal to size of Fj_names in KKSMultiFreeEnergy AuxKernel ",
               name());
  if (_gj_names.size() != _num_j)
    mooseError("Size of gj_names is not equal to size of Fj_names in KKSMultiFreeEnergy AuxKernel ",
               name());

  // get bulk properties
  for (unsigned int i = 0; i < _num_j; ++i)
  {
    _prop_Fj[i] = &getMaterialPropertyByName<Real>(_Fj_names[i]);
    _prop_hj[i] = &getMaterialPropertyByName<Real>(_hj_names[i]);
    _prop_gj[i] = &getMaterialPropertyByName<Real>(_gj_names[i]);
  }

  // Check to ensure size of interfacial_vars is the same as kappa_names
  if (_nvars != _nkappas)
    mooseError("Size of interfacial_vars is not equal to the size of kappa_names in "
               "KKSMultiFreeEnergy AuxKernel ",
               name());

  // Assign kappa values
  for (unsigned int i = 0; i < _nkappas; ++i)
    _kappas[i] = &getMaterialPropertyByName<Real>(_kappa_names[i]);
}

Real
KKSMultiFreeEnergy::computeValue()
{
  // Start with any additional energy contribution, which is 0 if not supplied
  Real total_energy = _additional_free_energy[_qp];
  // Add bulk energy contributions
  for (unsigned int i = 0; i < _num_j; ++i)
    total_energy += (*_prop_hj[i])[_qp] * (*_prop_Fj[i])[_qp] + _w * (*_prop_gj[i])[_qp];

  // Add interfacial energy of each variable
  for (unsigned int i = 0; i < _nvars; ++i)
    total_energy += (*_kappas[i])[_qp] / 2.0 * (*_grad_vars[i])[_qp].norm_sq();

  return total_energy;
}
