/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSACBulkBase.h"

template<>
InputParameters validParams<KKSACBulkBase>()
{
  InputParameters params = validParams<ACBulk>();
  params.addClassDescription("KKS model kernel for the Bulk Allen-Cahn. This operates on the order parameter 'eta' as the non-linear variable");
  params.addRequiredParam<MaterialPropertyName>("fa_name", "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addRequiredParam<MaterialPropertyName>("fb_name", "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addParam<MaterialPropertyName>("h_name", "h", "Base name for the switching function h(eta)");
  params.addCoupledVar("args", "coupled variables i.e. concentrations");
  return params;
}

KKSACBulkBase::KKSACBulkBase(const std::string & name, InputParameters parameters) :
    ACBulk(name, parameters),
    // number of coupled variables (ca, args_a[])
    _nvar(_coupled_moose_vars.size()),
    _eta_name(_var.name()),
    _Fa_name(getParam<MaterialPropertyName>("fa_name")),
    _Fb_name(getParam<MaterialPropertyName>("fb_name")),
    _h_name(getParam<MaterialPropertyName>("h_name")),
    _prop_Fa(getMaterialPropertyByName<Real>(_Fa_name)),
    _prop_Fb(getMaterialPropertyByName<Real>(_Fb_name)),
    _prop_dFa(getMaterialPropertyDerivative<Real>(_Fa_name, _eta_name)),
    _prop_dFb(getMaterialPropertyDerivative<Real>(_Fb_name, _eta_name)),
    _prop_dh(getMaterialPropertyDerivative<Real>(_h_name, _eta_name)),
    _prop_d2h(getMaterialPropertyDerivative<Real>(_h_name, _eta_name, _eta_name))
{
  // reserve space for derivatives
  _derivatives_Fa.resize(_nvar);
  _derivatives_Fb.resize(_nvar);
  _grad_args.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable *cvar = _coupled_moose_vars[i];

    // get the first derivatives of Fa and Fb material property
    _derivatives_Fa[i] = &getMaterialPropertyDerivative<Real>(_Fa_name, cvar->name());
    _derivatives_Fb[i] = &getMaterialPropertyDerivative<Real>(_Fb_name, cvar->name());

    // get the gradient
    _grad_args[i] = &(cvar->gradSln());
  }
}
