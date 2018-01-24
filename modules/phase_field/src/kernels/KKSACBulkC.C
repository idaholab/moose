/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSACBulkC.h"

template <>
InputParameters
validParams<KKSACBulkC>()
{
  InputParameters params = validParams<KKSACBulkBase>();
  params.addClassDescription("KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar("ca", "a-phase concentration");
  params.addRequiredCoupledVar("cb", "b-phase concentration");
  return params;
}

KKSACBulkC::KKSACBulkC(const InputParameters & parameters)
  : KKSACBulkBase(parameters),
    _ca_name(getVar("ca", 0)->name()),
    _ca_var(coupled("ca")),
    _ca(coupledValue("ca")),
    _cb_name(getVar("cb", 0)->name()),
    _cb_var(coupled("cb")),
    _cb(coupledValue("cb")),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _prop_dFadca(getMaterialPropertyDerivative<Real>("fa_name", _ca_name)),
    _prop_d2Fadca2(getMaterialPropertyDerivative<Real>("fa_name", _ca_name, _ca_name)),
    _prop_d2Fbdcb2(getMaterialPropertyDerivative<Real>("fb_name", _cb_name, _cb_name))
{
  // Resize to number of coupled variables (_nvar from KKSACBulkBase constructor)
  _prop_d2Fadcadarg.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable * cvar = _coupled_moose_vars[i];

    // get second partial derivatives wrt ca and other coupled variable
    _prop_d2Fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, cvar->name());
  }
}

Real
KKSACBulkC::computeDFDOP(PFFunctionType type)
{
  Real res = 0.0;
  Real A1 = _prop_dFadca[_qp] * (_ca[_qp] - _cb[_qp]);

  switch (type)
  {
    case Residual:
      return _prop_dh[_qp] * A1;

    case Jacobian:
      res = _prop_d2h[_qp] * A1;

      return _phi[_j][_qp] * res;
  }

  mooseError("Invalid type passed in");
}

Real
KKSACBulkC::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);
  // Then add dependence of KKSACBulkF on other variables
  // Treat ca and cb specially, as they appear in the residual
  if (jvar == _ca_var)
  {
    res += _L[_qp] * _prop_dh[_qp] *
           ((_ca[_qp] - _cb[_qp]) * _prop_d2Fadca2[_qp] + _prop_dFadca[_qp]) * _phi[_j][_qp] *
           _test[_i][_qp];

    return res;
  }

  if (jvar == _cb_var)
  {
    res -= _L[_qp] * _prop_dh[_qp] * _prop_dFadca[_qp] * _phi[_j][_qp] * _test[_i][_qp];

    return res;
  }

  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  res += _L[_qp] * _prop_dh[_qp] * (*_prop_d2Fadcadarg[cvar])[_qp] * (_ca[_qp] - _cb[_qp]) *
         _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
