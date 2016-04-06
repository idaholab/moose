/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSMultiACBulkBase.h"

template<>
InputParameters validParams<KKSMultiACBulkBase>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Multi-order parameter KKS model kernel for the Bulk Allen-Cahn. This operates on one of the order parameters 'eta_i' as the non-linear variable");
  params.addRequiredParam<std::vector<MaterialPropertyName> >("fj_names", "List of free energies for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName> >("hj_names", "Switching Function Materials that provide h. Place in same order as fj_names!");
  return params;
}

KKSMultiACBulkBase::KKSMultiACBulkBase(const InputParameters & parameters) :
    ACBulk<Real>(parameters),
    // number of coupled variables (ca, args_a[])
    //_nvar(_coupled_moose_vars.size()),
    _etai_name(_var.name()),
    _fj_names(getParam<std::vector<MaterialPropertyName> >("fj_names")),
    _num_fj(_fj_names.size()),
    _prop_Fj(_num_fi),
    _hj_names(getParam<std::vector<MaterialPropertyName> >("hj_names")),
    _num_hj(_hj_names.size()),
    _prop_hj(_num_hj),
    _prop_dhjdetai(_num_hj),
    _prop_d2hjdetaidetaj(_num_hj)
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
    _derivatives_Fa[i] = &getMaterialPropertyDerivative<Real>("fa_name", cvar->name());
    _derivatives_Fb[i] = &getMaterialPropertyDerivative<Real>("fb_name", cvar->name());

    // get the gradient
    _grad_args[i] = &(cvar->gradSln());
  }
}

void
KKSMultiACBulkBase::initialSetup()
{
  ACBulk<Real>::initialSetup();
  validateNonlinearCoupling<Real>("fa_name");
  validateNonlinearCoupling<Real>("fb_name");
}
