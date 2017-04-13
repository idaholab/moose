/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSACBulkF.h"

template <>
InputParameters
validParams<KKSACBulkF>()
{
  InputParameters params = validParams<KKSACBulkBase>();
  params.addClassDescription("KKS model kernel (part 1 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms NOT dependent on chemical potential.");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  params.addParam<MaterialPropertyName>(
      "g_name", "g", "Base name for the double well function g(eta)");
  return params;
}

KKSACBulkF::KKSACBulkF(const InputParameters & parameters)
  : KKSACBulkBase(parameters),
    _w(getParam<Real>("w")),
    _prop_dg(getMaterialPropertyDerivative<Real>("g_name", _eta_name)),
    _prop_d2g(getMaterialPropertyDerivative<Real>("g_name", _eta_name, _eta_name))
{
}

Real
KKSACBulkF::computeDFDOP(PFFunctionType type)
{
  Real res = 0.0;
  Real A1 = _prop_Fa[_qp] - _prop_Fb[_qp];

  switch (type)
  {
    case Residual:
      return -_prop_dh[_qp] * A1 + _w * _prop_dg[_qp];

    case Jacobian:
    {
      res = -_prop_d2h[_qp] * A1 + _w * _prop_d2g[_qp];

      return _phi[_j][_qp] * res;
    }
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

  // Then add dependence of KKSACBulkF on other variables
  res -= _L[_qp] * _prop_dh[_qp] * ((*_derivatives_Fa[cvar])[_qp] - (*_derivatives_Fb[cvar])[_qp]) *
         _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
