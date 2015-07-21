/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSACBulkF.h"

template<>
InputParameters validParams<KKSACBulkF>()
{
  InputParameters params = validParams<KKSACBulkBase>();
  // params.addClassDescription("KKS model kernel for the Bulk Allen-Cahn. This operates on the order parameter 'eta' as the non-linear variable");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  params.addParam<MaterialPropertyName>("g_name", "g", "Base name for the double well function g(eta)");
  return params;
}

KKSACBulkF::KKSACBulkF(const InputParameters & parameters) :
    KKSACBulkBase(parameters),
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
      res =  -_prop_d2h[_qp] * A1
            + _w * _prop_d2g[_qp];

      // the -\frac{dh}{d\eta}\left(\frac{dF_a}{d\eta}-\frac{dF_b}{d\eta}\right)
      // term is handled in KKSACBulkC!
      return _phi[_j][_qp] * res;
    }
  }

  mooseError("Invalid type passed in");
}

Real
KKSACBulkF::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  Real res = _prop_dh[_qp] * (  (*_derivatives_Fa[cvar])[_qp]
                              - (*_derivatives_Fb[cvar])[_qp])
                           * _phi[_j][_qp];
  return res * _test[_j][_qp];
}


// DEPRECATED CONSTRUCTOR
KKSACBulkF::KKSACBulkF(const std::string & deprecated_name, InputParameters parameters) :
    KKSACBulkBase(deprecated_name, parameters),
    _w(getParam<Real>("w")),
    _prop_dg(getMaterialPropertyDerivative<Real>("g_name", _eta_name)),
    _prop_d2g(getMaterialPropertyDerivative<Real>("g_name", _eta_name, _eta_name))
{
}
