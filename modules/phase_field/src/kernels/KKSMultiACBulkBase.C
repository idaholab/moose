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
  params.addRequiredParam<std::vector<MaterialPropertyName> >("Fj_names", "List of free energies for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName> >("hj_names", "Switching Function Materials that provide h. Place in same order as Fj_names!");
  return params;
}

KKSMultiACBulkBase::KKSMultiACBulkBase(const InputParameters & parameters) :
    ACBulk<Real>(parameters),
    _nvar(_coupled_moose_vars.size()), // number of coupled variables
    _etai_name(_var.name()),
    _Fj_names(getParam<std::vector<MaterialPropertyName> >("Fj_names")),
    _num_Fj(_Fj_names.size()),
    _prop_Fj(_num_Fj),
    _hj_names(getParam<std::vector<MaterialPropertyName> >("hj_names")),
    _num_hj(_hj_names.size()),
    _prop_hj(_num_hj),
    _prop_dhjdetai(_num_hj),
    _prop_d2hjdetai2(_num_hj),
    _prop_d2hjdetaidarg(_num_hj)
{
  // check passed in parameter vectors
  if (_num_Fj != _num_hj)
    mooseError("Need to pass in as many hj_names as Fj_names in KKSMultiACBulkF and KKSMultiACBulkC " << name());

  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_Fj; ++n)
  {
    // get phase free energy
    _prop_Fj[n] = &getMaterialPropertyByName<Real>(_Fj_names[n]);
    // _prop_dFi[n].resize(_nargs);
    // _prop_d2Fi[n].resize(_nargs);
    // _prop_d3Fi[n].resize(_nargs);
    //

    // get switching function and derivatives wrt eta_i, the nonlinear variable
    _prop_hj[n] = &getMaterialPropertyByName<Real>(_hj_names[n]);
    _prop_dhjdetai[n] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name);
    _prop_d2hjdetai2[n] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, _etai_name);
    _prop_d2hjdetaidarg[n].resize(_nvar);

    // Get second derivatives wrt eta_i and all coupled variables
    for (unsigned int i = 0; i < _nvar; ++i)
    {
      MooseVariable *cvar = _coupled_moose_vars[i];

      _prop_d2hjdetaidarg[n][i] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, cvar->name());

    //   _prop_dFi[n][i] = &getMaterialPropertyDerivative<Real>(_fi_names[n], _arg_names[i]);
    //   _prop_d2Fi[n][i].resize(_nargs);
    //
    //   if (_third_derivatives)
    //     _prop_d3Fi[n][i].resize(_nargs);
    //
    //   for (unsigned int j = 0; j < _nargs; ++j)
    //   {
    //     _prop_d2Fi[n][i][j] = &getMaterialPropertyDerivative<Real>(_fi_names[n], _arg_names[i], _arg_names[j]);
    //
    //     if (_third_derivatives) {
    //       _prop_d3Fi[n][i][j].resize(_nargs);
    //
    //       for (unsigned int k = 0; k < _nargs; ++k)
    //         _prop_d3Fi[n][i][j][k] = &getMaterialPropertyDerivative<Real>(_fi_names[n], _arg_names[i], _arg_names[j], _arg_names[k]);
    //     }
    //   }
    }
  }


  // reserve space for derivatives
  // _derivatives_Fa.resize(_nvar);
  // _derivatives_Fb.resize(_nvar);
  // _grad_args.resize(_nvar);

  // Iterate over all coupled variables
  // for (unsigned int i = 0; i < _nvar; ++i)
  // {
  //   MooseVariable *cvar = _coupled_moose_vars[i];
  //
  //   // get the first derivatives of Fa and Fb material property
  //   _derivatives_Fa[i] = &getMaterialPropertyDerivative<Real>("fa_name", cvar->name());
  //   _derivatives_Fb[i] = &getMaterialPropertyDerivative<Real>("fb_name", cvar->name());
  //
  //   // get the gradient
  //   _grad_args[i] = &(cvar->gradSln());
  // }
}

void
KKSMultiACBulkBase::initialSetup()
{
  ACBulk<Real>::initialSetup();
  // validateNonlinearCoupling<Real>("fa_name");
  // validateNonlinearCoupling<Real>("fb_name");
}
