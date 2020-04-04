//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSACBulkF.h"

registerMooseObject("PhaseFieldApp", KKSACBulkF);

InputParameters
KKSACBulkF::validParams()
{
  InputParameters params = KKSACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 1 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms NOT dependent on chemical potential.");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  params.addParam<MaterialPropertyName>(
      "g_name", "g", "Base name for the double well function g(eta)");
  params.addRequiredParam<MaterialPropertyName>(
      "fb_name",
      "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  return params;
}

KKSACBulkF::KKSACBulkF(const InputParameters & parameters)
  : KKSACBulkBase(parameters),
    _w(getParam<Real>("w")),
    _prop_dg(getMaterialPropertyDerivative<Real>("g_name", _eta_name)),
    _prop_d2g(getMaterialPropertyDerivative<Real>("g_name", _eta_name, _eta_name)),
    _prop_Fb(getMaterialProperty<Real>("fb_name")),
    _prop_dFb(getMaterialPropertyDerivative<Real>("fb_name", _eta_name))
{
}

Real
KKSACBulkF::computeDFDOP(PFFunctionType type)
{
  const Real A1 = _prop_Fa[_qp] - _prop_Fb[_qp];
  switch (type)
  {
    case Residual:
      return -_prop_dh[_qp] * A1 + _w * _prop_dg[_qp];

    case Jacobian:
      return _phi[_j][_qp] * (-_prop_d2h[_qp] * A1 + _w * _prop_d2g[_qp]);
  }

  mooseError("Invalid type passed in");
}

Real
KKSACBulkF::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  return res - _L[_qp] * _prop_dh[_qp] *
                   ((*_derivatives_Fa[cvar])[_qp] - (*_derivatives_Fb[cvar])[_qp]) * _phi[_j][_qp] *
                   _test[_i][_qp];
}
