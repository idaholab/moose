//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSMultiACBulkBase.h"

InputParameters
KKSMultiACBulkBase::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
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

KKSMultiACBulkBase::KKSMultiACBulkBase(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _etai_name(coupledName("eta_i", 0)),
    _etai_var(coupled("eta_i", 0)),
    _Fj_names(getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _prop_dFjdarg(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_num_j),
    _prop_dhjdetai(_num_j),
    _prop_d2hjdetai2(_num_j),
    _prop_d2hjdetaidarg(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != _hj_names.size())
    paramError("hj_names", "Need to pass in as many hj_names as Fj_names");

  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    // get phase free energy
    _prop_Fj[n] = &getMaterialPropertyByName<Real>(_Fj_names[n]);
    _prop_dFjdarg[n].resize(_n_args);

    // get switching function and derivatives wrt eta_i, the nonlinear variable
    _prop_hj[n] = &getMaterialPropertyByName<Real>(_hj_names[n]);
    _prop_dhjdetai[n] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name);
    _prop_d2hjdetai2[n] =
        &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, _etai_name);
    _prop_d2hjdetaidarg[n].resize(_n_args);

    for (unsigned int i = 0; i < _n_args; ++i)
    {
      // Get derivatives of all Fj wrt all coupled variables
      _prop_dFjdarg[n][i] = &getMaterialPropertyDerivative<Real>(_Fj_names[n], i);

      // Get second derivatives of all hj wrt eta_i and all coupled variables
      _prop_d2hjdetaidarg[n][i] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, i);
    }
  }
}

void
KKSMultiACBulkBase::initialSetup()
{
  ACBulk<Real>::initialSetup();

  for (unsigned int n = 0; n < _num_j; ++n)
  {
    validateNonlinearCoupling<Real>(_Fj_names[n]);
    validateNonlinearCoupling<Real>(_hj_names[n]);
  }
}
