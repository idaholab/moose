#include "KKSACBulkBase.h"

template<>
InputParameters validParams<KKSACBulkBase>()
{
  InputParameters params = validParams<ACBulk>();
  params.addClassDescription("KKS model kernel for the Bulk Allen-Cahn. This operates on the order parameter 'eta' as the non-linear variable");
  params.addRequiredParam<std::string>("fa_name", "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addRequiredParam<std::string>("fb_name", "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");

  params.addParam<std::string>("h_name", "h", "Base name for the switching function h(eta)");
  return params;
}

KKSACBulkBase::KKSACBulkBase(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<ACBulk> >(name, parameters),
    // number of coupled variables (ca, args_a[])
    _nvar(_coupled_moose_vars.size()),
    _eta_name(_var.name()),
    _Fa_name(getParam<std::string>("fa_name")),
    _Fb_name(getParam<std::string>("fa_name")),
    _h_name(getParam<std::string>("h_name")),
    _prop_Fa(getMaterialProperty<Real>(_Fa_name)),
    _prop_Fb(getMaterialProperty<Real>(_Fb_name)),
    _prop_dFa(getDerivative<Real>(_Fa_name, _eta_name)),
    _prop_dFb(getDerivative<Real>(_Fb_name, _eta_name)),
    _prop_dh(getDerivative<Real>(_h_name, _eta_name)),
    _prop_d2h(getDerivative<Real>(_h_name, _eta_name, _eta_name))
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
    _derivatives_Fa[i] = &getDerivative<Real>(_Fa_name, cvar->name());
    _derivatives_Fb[i] = &getDerivative<Real>(_Fb_name, cvar->name());

    // get the gradient
    _grad_args[i] = &(cvar->gradSln());
  }
}
