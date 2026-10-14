//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKKSMultiACBulkBase.h"

InputParameters
ADKKSMultiACBulkBase::validParams()
{
  InputParameters params = ADAllenCahnBase<Real>::validParams();
  params.addClassDescription("Multi-order parameter KKS model kernel for the Bulk Allen-Cahn. This "
                             "operates on one of the order parameters 'eta_i' as the non-linear "
                             "variable");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Fj_names", "List of free energies for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide h. Place in same order as Fj_names!");
  params.addRequiredCoupledVar("eta_i",
                               "Order parameter that derivatives are taken with respect to");
  return params;
}

ADKKSMultiACBulkBase::ADKKSMultiACBulkBase(const InputParameters & parameters)
  : ADAllenCahnBase<Real>(parameters),
    _etai_name(coupledName("eta_i", 0)),
    _Fj_names(getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_dhjdetai(_num_j)
{
  if (_num_j != _hj_names.size())
    paramError("hj_names", "Need to pass in as many hj_names as Fj_names");

  for (unsigned int n = 0; n < _num_j; ++n)
  {
    _prop_Fj[n] = &getADMaterialPropertyByName<Real>(_Fj_names[n]);
    _prop_dhjdetai[n] = &getMaterialPropertyDerivativeByName<Real, true>(_hj_names[n], _etai_name);
  }
}

void
ADKKSMultiACBulkBase::initialSetup()
{
  ADAllenCahnBase<Real>::initialSetup();

  for (unsigned int n = 0; n < _num_j; ++n)
  {
    validateNonlinearCoupling<ADReal>(_Fj_names[n]);
    validateNonlinearCoupling<ADReal>(_hj_names[n]);
  }
}
