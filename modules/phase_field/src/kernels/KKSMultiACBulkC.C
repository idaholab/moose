/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSMultiACBulkC.h"

template<>
InputParameters validParams<KKSMultiACBulkC>()
{
  InputParameters params = validParams<KKSMultiACBulkBase>();
  params.addClassDescription("Multi-phase KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. This includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar("cj", "Array of phase concentrations cj. Place in same order as Fj_names!");
  return params;
}

KKSMultiACBulkC::KKSMultiACBulkC(const InputParameters & parameters) :
    KKSMultiACBulkBase(parameters),
    _ncj(coupledComponents("cj")),
    //_cj_names(_ncj),
    _c1_name(getVar("cj", 0)->name()),
    _cjs(_ncj),
    _cjs_var(_ncj),
    // _ca_name(getVar("ca", 0)->name()),
    // _ca_var(coupled("ca")),
    // _ca(coupledValue("ca")),
    // _cb_name(getVar("cb", 0)->name()),
    // _cb_var(coupled("cb")),
    // _cb(coupledValue("cb")),
    // _prop_h(getMaterialProperty<Real>("h")),
    _prop_dF1dc1(getMaterialPropertyDerivative<Real>(_Fj_names[0], _c1_name))
    // _prop_d2Fadca2(getMaterialPropertyDerivative<Real>("fa_name", _ca_name, _ca_name)),
    // _prop_d2Fbdcb2(getMaterialPropertyDerivative<Real>("fb_name", _cb_name, _cb_name))
{
  // Load concentration variables into the arrays
  for (unsigned int i = 0; i < _ncj; ++i)
  {
    //_cj_names[i] = getVar("cj", i)->name();
    _cjs[i] = &coupledValue("cj", i);
    _cjs_var[i] = coupled("cj", i);
  }

  //_prop_dF1dc1 = getMaterialPropertyDerivative<Real>(_Fj_names[0], _cj_names[0]);
  // //Resize to number of coupled variables (_nvar from KKSACBulkBase constructor)
  // _prop_d2Fadcadarg.resize(_nvar);
  //
  // // Iterate over all coupled variables
  // for (unsigned int i = 0; i < _nvar; ++i)
  // {
  //   MooseVariable *cvar = _coupled_moose_vars[i];
  //
  //   // get second partial derivatives wrt ca and other coupled variable
  //   _prop_d2Fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, cvar->name());
  // }
}

Real
KKSMultiACBulkC::computeDFDOP(PFFunctionType type)
{
  Real res = 0.0;
  // Real A1 = _prop_dFadca[_qp] * (_ca[_qp] - _cb[_qp]);

  switch (type)
  {
    case Residual:
    {
      for (unsigned int n = 0; n < _ncj; ++n)
        res += (*_prop_dhjdetai[n])[_qp] * (*_cjs[n])[_qp];

      return - _prop_dF1dc1[_qp] * res;
    }

    case Jacobian:
    {
      // res =   _prop_d2h[_qp] * A1;
      //
      return res;

    }
  }

  mooseError("Invalid type passed in");
}

Real
KKSMultiACBulkC::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);
  // // Then add dependence of KKSACBulkF on other variables
  // // Treat ca and cb specially, as they appear in the residual
  // if (jvar == _ca_var)
  // {
  //   res += _L[_qp] * _prop_dh[_qp] * (
  //       (_ca[_qp] - _cb[_qp]) * _prop_d2Fadca2[_qp] + _prop_dFadca[_qp]
  //       ) * _phi[_j][_qp] * _test[_i][_qp];
  //
  //   return res;
  // }
  //
  // if (jvar == _cb_var)
  // {
  //   res -= _L[_qp] * _prop_dh[_qp] * _prop_dFadca[_qp]
  //             * _phi[_j][_qp] * _test[_i][_qp];
  //
  //   return res;
  // }
  //
  // //  for all other vars get the coupled variable jvar is referring to
  // unsigned int cvar;
  // if (!mapJvarToCvar(jvar, cvar))
  //   return res;
  //
  // res += _L[_qp] * _prop_dh[_qp] * (*_prop_d2Fadcadarg[cvar])[_qp]
  //           * (_ca[_qp] - _cb[_qp]) * _phi[_j][_qp]  * _test[_i][_qp];
  //
  return res;
}
