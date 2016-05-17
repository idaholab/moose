/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSMultiPhaseConcentration.h"

template<>
InputParameters validParams<KKSMultiPhaseConcentration>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("KKS multi-phase model kernel to enforce (c = h1*c1 + h2*c2 + h3*c3 +.. The non-linear variable of this kernel is cn, the final phase concenration in the list.");
  params.addRequiredCoupledVar("cj", "Array of phase concentrations cj. Place in same order as hj_names!");
  params.addRequiredCoupledVar("c", "Physical concentration");
  params.addCoupledVar("etas", "Order parameters for all phases");
  params.addRequiredParam<std::vector<MaterialPropertyName> >("hj_names", "Switching Function Materials that provide h(eta_1, eta_2,...)");
  return params;
}

// Phase interpolation func
KKSMultiPhaseConcentration::KKSMultiPhaseConcentration(const InputParameters & parameters) :
    DerivativeMaterialInterface<Kernel>(parameters),
    _ncj(coupledComponents("cj")),
    _cjs(_ncj),
    _cjs_var(_ncj),
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _hj_names(getParam<std::vector<MaterialPropertyName> >("hj_names")),
    _nhj(_hj_names.size()),
    _prop_hj(_nhj),
    _num_etas(coupledComponents("etas")),
    _eta_names(_num_etas),
    _eta_vars(_num_etas),
    _prop_dhjdetai(_nhj)
{
  //Check to make sure the the number of cj's is the same as the number of hj's
  if (_ncj != _nhj)
    mooseError("Need to pass in as many hj_names as cj_names in KKSMultiPhaseConcentration " << name());
  //Check to make sure the the number of etas is the same as the number of hj's
  if (_num_etas != _nhj)
    mooseError("Need to pass in as many hj_names as etas in KKSMultiPhaseConcentration " << name());

  // get order parameter names and variable indices
  for (unsigned int i = 0; i < _num_etas; ++i)
  {
    _eta_names[i] = getVar("etas", i)->name();
    _eta_vars[i] = coupled("etas", i);
  }

  // Load concentration variables into the arrays
  for (unsigned int m = 0; m < _ncj; ++m)
  {
    //_cj_names[i] = getVar("cj", i)->name();
    _cjs[m] = &coupledValue("cj", m);
    _cjs_var[m] = coupled("cj", m);
    _prop_hj[m] = &getMaterialPropertyByName<Real>(_hj_names[m]);
    _prop_dhjdetai[m].resize(_nhj);

    // Get derivatives of switching functions wrt order parameters
    for (unsigned int n = 0; n < _num_etas; ++n)
      _prop_dhjdetai[m][n] = &getMaterialPropertyDerivative<Real>(_hj_names[m], _eta_names[n]);
  }
}

Real
KKSMultiPhaseConcentration::computeQpResidual()
{
  // R = sum_i (h_i * c_i) - c
  Real sum_ch = 0.0;
  for (unsigned int m = 0; m < _ncj; ++m)
    sum_ch += (*_cjs[m])[_qp] * (*_prop_hj[m])[_qp];

  return _test[_i][_qp] * (sum_ch - _c[_qp]);
}

Real
KKSMultiPhaseConcentration::computeQpJacobian()
{
  // The last phase concentration is the nonlinear variable
  return _test[_i][_qp] * (*_prop_hj[_ncj - 1])[_qp] * _phi[_j][_qp];
}

Real
KKSMultiPhaseConcentration::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return -_test[_i][_qp] * _phi[_j][_qp];

  for (unsigned int m = 0; m < _ncj - 1; ++m)
    if (jvar == _cjs_var[m])
      return _test[_i][_qp] * (*_prop_hj[m])[_qp] * _phi[_j][_qp];

  for (unsigned int m = 0; m < _ncj; ++m)
    if (jvar == _eta_vars[m])
    {
      Real sum = 0.0;

      for (unsigned int n = 0; n < _ncj; ++n)
        sum += (*_prop_dhjdetai[n][m])[_qp] * (*_cjs[n])[_qp];

      return _test[_i][_qp] * sum * _phi[_j][_qp];
    }

  return 0.0;
}
