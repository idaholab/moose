//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSACBulkC.h"

registerMooseObject("PhaseFieldApp", KKSACBulkC);

InputParameters
KKSACBulkC::validParams()
{
  InputParameters params = KKSACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar("ca", "a-phase concentration");
  params.addRequiredCoupledVar("cb", "b-phase concentration");
  return params;
}

KKSACBulkC::KKSACBulkC(const InputParameters & parameters)
  : KKSACBulkBase(parameters),
    _ca_name(coupledName("ca", 0)),
    _ca_var(coupled("ca")),
    _ca(coupledValue("ca")),
    _cb_name(coupledName("cb", 0)),
    _cb_var(coupled("cb")),
    _cb(coupledValue("cb")),
    _prop_dFadca(getMaterialPropertyDerivative<Real>("fa_name", _ca_name)),
    _prop_d2Fadca2(getMaterialPropertyDerivative<Real>("fa_name", _ca_name, _ca_name)),
    _prop_d2Fadcadarg(_n_args)
{
  // get second partial derivatives wrt ca and other coupled variable
  for (unsigned int i = 0; i < _n_args; ++i)
    _prop_d2Fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, i);
}

Real
KKSACBulkC::computeDFDOP(PFFunctionType type)
{
  const Real A1 = _prop_dFadca[_qp] * (_ca[_qp] - _cb[_qp]);
  switch (type)
  {
    case Residual:
      return _prop_dh[_qp] * A1;

    case Jacobian:
      return _phi[_j][_qp] * _prop_d2h[_qp] * A1;
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
    return res + _L[_qp] * _prop_dh[_qp] *
                     ((_ca[_qp] - _cb[_qp]) * _prop_d2Fadca2[_qp] + _prop_dFadca[_qp]) *
                     _phi[_j][_qp] * _test[_i][_qp];

  if (jvar == _cb_var)
    return res - _L[_qp] * _prop_dh[_qp] * _prop_dFadca[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  res += _L[_qp] * _prop_dh[_qp] * (*_prop_d2Fadcadarg[cvar])[_qp] * (_ca[_qp] - _cb[_qp]) *
         _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
