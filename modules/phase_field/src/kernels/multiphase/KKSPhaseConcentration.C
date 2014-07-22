#include "KKSPhaseConcentration.h"

template<>
InputParameters validParams<KKSPhaseConcentration>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("KKS model kernel to enforce the decomposition of concentration into phase concentration  (1-h(eta))*ca + h(eta)*cb - c = 0. The non-linear variable of this kernel is cb.");
  params.addRequiredCoupledVar("ca", "Phase a concentration");
  params.addRequiredCoupledVar("c", "Real concentration");
  params.addRequiredCoupledVar("eta", "Phase a/b order parameter");
  params.addParam<std::string>("h_name", "h", "Base name for the switching function h(eta)");
  return params;
}

// Phase interpolation func
KKSPhaseConcentration::KKSPhaseConcentration(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<Kernel>(name, parameters),
    _ca(coupledValue("ca")),
    _ca_var(coupled("ca")),
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _eta(coupledValue("eta")),
    _eta_var(coupled("eta")),
    _h_name(getParam<std::string>("h_name")),
    _prop_h(getMaterialProperty<Real>(_h_name)),
    _prop_dh(getDerivative<Real>(_h_name, getVar("eta", 0)->name()))
{
}

Real
KKSPhaseConcentration::computeQpResidual()
{
  // R = (1-h(eta))*ca + h(eta)*cb - c
  return _test[_i][_qp] * (
             (1.0 - _prop_h[_qp]) * _ca[_qp]
           + _prop_h[_qp] * _u[_qp]
           - _c[_qp]
         );
}

Real
KKSPhaseConcentration::computeQpJacobian()
{
  return _test[_i][_qp] * _prop_h[_qp] * _phi[_j][_qp];
}

Real
KKSPhaseConcentration::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _ca_var)
    return _test[_i][_qp] * (1.0 - _prop_h[_qp]) * _phi[_j][_qp];

  else if (jvar == _c_var)
    return -_test[_i][_qp] * _phi[_j][_qp];

  else if (jvar == _eta_var)
    return _test[_i][_qp] * (_u[_qp] - _ca[_qp]) * _prop_dh[_qp] * _phi[_j][_qp];

  return 0.0;
}
