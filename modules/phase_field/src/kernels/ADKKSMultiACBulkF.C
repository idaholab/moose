//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKKSMultiACBulkF.h"

registerMooseObject("PhaseFieldApp", ADKKSMultiACBulkF);

InputParameters
ADKKSMultiACBulkF::validParams()
{
  InputParameters params = ADKKSMultiACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 1 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms NOT dependent on chemical potential.");
  params.addRequiredParam<Real>("wi", "Double well height parameter");
  params.addRequiredParam<MaterialPropertyName>(
      "gi_name", "Base name for the double well function g_i(eta_i)");
  return params;
}

ADKKSMultiACBulkF::ADKKSMultiACBulkF(const InputParameters & parameters)
  : ADKKSMultiACBulkBase(parameters),
    _wi(getParam<Real>("wi")),
    _gi_name(getParam<MaterialPropertyName>("gi_name")),
    _prop_dgi(getADMaterialPropertyByName<Real>(derivativePropertyNameFirst(_gi_name, _etai_name)))
{
}

ADReal
ADKKSMultiACBulkF::computeDFDOP()
{
  ADReal sum = 0;

  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];

  return sum + _wi * _prop_dgi[_qp];
}
