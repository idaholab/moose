//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSACBulkBase.h"

template <>
InputParameters
validParams<KKSACBulkBase>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("KKS model kernel for the Bulk Allen-Cahn. This operates on the order "
                             "parameter 'eta' as the non-linear variable");
  params.addRequiredParam<MaterialPropertyName>(
      "fa_name",
      "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addRequiredParam<MaterialPropertyName>(
      "fb_name",
      "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addParam<MaterialPropertyName>(
      "h_name", "h", "Base name for the switching function h(eta)");
  return params;
}

KKSACBulkBase::KKSACBulkBase(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    // number of coupled variables (ca, args_a[])
    _nvar(_coupled_moose_vars.size()),
    _eta_name(_var.name()),
    _prop_Fa(getMaterialProperty<Real>("fa_name")),
    _prop_Fb(getMaterialProperty<Real>("fb_name")),
    _prop_dFa(getMaterialPropertyDerivative<Real>("fa_name", _eta_name)),
    _prop_dFb(getMaterialPropertyDerivative<Real>("fb_name", _eta_name)),
    _prop_dh(getMaterialPropertyDerivative<Real>("h_name", _eta_name)),
    _prop_d2h(getMaterialPropertyDerivative<Real>("h_name", _eta_name, _eta_name))
{
  // reserve space for derivatives
  _derivatives_Fa.resize(_nvar);
  _derivatives_Fb.resize(_nvar);
  _grad_args.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable * cvar = _coupled_moose_vars[i];

    // get the first derivatives of Fa and Fb material property
    _derivatives_Fa[i] = &getMaterialPropertyDerivative<Real>("fa_name", cvar->name());
    _derivatives_Fb[i] = &getMaterialPropertyDerivative<Real>("fb_name", cvar->name());

    // get the gradient
    _grad_args[i] = &(cvar->gradSln());
  }
}

void
KKSACBulkBase::initialSetup()
{
  ACBulk<Real>::initialSetup();
  validateNonlinearCoupling<Real>("fa_name");
  validateNonlinearCoupling<Real>("fb_name");
}
