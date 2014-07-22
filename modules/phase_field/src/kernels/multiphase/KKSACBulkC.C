#include "KKSACBulkC.h"

template<>
InputParameters validParams<KKSACBulkC>()
{
  InputParameters params = validParams<KKSACBulkBase>();
  //params.addClassDescription("KKS model kernel for the Bulk Allen-Cahn. This operates on the order parameter 'eta' as the non-linear variable");
  params.addRequiredCoupledVar("ca", "a-phase concentration");
  params.addRequiredCoupledVar("cb", "b-phase concentration");
  return params;
}

KKSACBulkC::KKSACBulkC(const std::string & name, InputParameters parameters) :
    KKSACBulkBase(name, parameters),
    _ca_name(getVar("ca", 0)->name()),
    _ca(coupledValue("ca")),
    _cb_name(getVar("cb", 0)->name()),
    _cb(coupledValue("cb")),
    _prop_h(getMaterialProperty<Real>(_h_name)),
    _prop_dFadca(getDerivative<Real>(_Fa_name, _ca_name)),
    _prop_d2Fadca2(getDerivative<Real>(_Fa_name, _ca_name, _ca_name)),
    _prop_d2Fbdcb2(getDerivative<Real>(_Fb_name, _cb_name, _cb_name))
{
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
    {
      // Eq. (25) in the KKS paper
      Real dcadeta =   _prop_dh[_qp] * (_ca[_qp] - _cb[_qp]) * _prop_d2Fbdcb2[_qp]
                     / ((1.0 - _prop_h[_qp]) * _prop_d2Fbdcb2[_qp] + _prop_h[_qp] * _prop_d2Fadca2[_qp]);

      Real A2 = (_ca[_qp] - _cb[_qp]) * _prop_d2Fadca2[_qp] * dcadeta;

      res =   _prop_d2h[_qp] * A1
            + _prop_dh[_qp]  * A2;

      return _phi[_j][_qp] * res;
    }
  }

  mooseError("Invalid type passed in");
}

Real
KKSACBulkC::computeQpOffDiagJacobian(unsigned int jvar)
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
