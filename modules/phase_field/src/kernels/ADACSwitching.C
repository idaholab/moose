//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADACSwitching.h"

registerMooseObject("PhaseFieldApp", ADACSwitching);

InputParameters
ADACSwitching::validParams()
{
  InputParameters params = ADAllenCahnBase<Real>::validParams();
  params.addClassDescription(
      "Kernel for Allen-Cahn equation that adds derivatives of switching functions and energies");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Fj_names", "List of free energies for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide h. Place in same order as Fj_names!");
  return params;
}

ADACSwitching::ADACSwitching(const InputParameters & parameters)
  : ADAllenCahnBase<Real>(parameters),
    _etai_name(_var.name()),
    _Fj_names(getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_dhjdetai(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != _hj_names.size())
    paramError("hj_names", "Need to pass in as many hj_names as Fj_names");

  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    // get phase free energy
    _prop_Fj[n] = &getADMaterialProperty<Real>(_Fj_names[n]);

    // get switching derivatives wrt eta_i, the nonlinear variable
    _prop_dhjdetai[n] =
        &getADMaterialProperty<Real>(derivativePropertyNameFirst(_hj_names[n], _etai_name));
  }
}

ADReal
ADACSwitching::computeDFDOP()
{
  ADReal sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];
  return sum;
}
