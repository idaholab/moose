//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKKSMultiACBulkC.h"

registerMooseObject("PhaseFieldApp", ADKKSMultiACBulkC);

InputParameters
ADKKSMultiACBulkC::validParams()
{
  InputParameters params = ADKKSMultiACBulkBase::validParams();
  params.addClassDescription("Multi-phase KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. "
                             "This includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar(
      "cj_names", "Array of phase concentrations cj. Place in same order as Fj_names!");
  return params;
}

ADKKSMultiACBulkC::ADKKSMultiACBulkC(const InputParameters & parameters)
  : ADKKSMultiACBulkBase(parameters),
    _c1_name(coupledName("cj_names", 0)),
    _cjs(adCoupledValues("cj_names")),
    _prop_dF1dc1(
        getADMaterialPropertyByName<Real>(derivativePropertyNameFirst(_Fj_names[0], _c1_name)))
{
  if (_num_j != coupledComponents("cj_names"))
    paramError("cj_names", "Need to pass in as many cj_names as Fj_names");
}

ADReal
ADKKSMultiACBulkC::computeDFDOP()
{
  ADReal sum = 0;

  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_dhjdetai[n])[_qp] * (*_cjs[n])[_qp];

  return -_prop_dF1dc1[_qp] * sum;
}
