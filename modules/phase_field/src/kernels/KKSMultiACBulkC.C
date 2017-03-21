/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSMultiACBulkC.h"

template <>
InputParameters
validParams<KKSMultiACBulkC>()
{
  InputParameters params = validParams<KKSMultiACBulkBase>();
  params.addClassDescription("Multi-phase KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. "
                             "This includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar(
      "cj_names", "Array of phase concentrations cj. Place in same order as Fj_names!");
  return params;
}

KKSMultiACBulkC::KKSMultiACBulkC(const InputParameters & parameters)
  : KKSMultiACBulkBase(parameters),
    _c1_name(getVar("cj_names", 0)
                 ->name()), // Can use any dFj/dcj since they are equal so pick first cj in the list
    _cjs(_num_j),
    _cjs_var(_num_j),
    _prop_dF1dc1(getMaterialPropertyDerivative<Real>(_Fj_names[0],
                                                     _c1_name)), // Use first Fj in list for dFj/dcj
    _prop_d2F1dc12(getMaterialPropertyDerivative<Real>(_Fj_names[0], _c1_name, _c1_name))
{
  if (_num_j != coupledComponents("cj_names"))
    mooseError("Need to pass in as many cj_names as Fj_names in KKSMultiACBulkC ", name());

  // Load concentration variables into the arrays
  for (unsigned int i = 0; i < _num_j; ++i)
  {
    _cjs[i] = &coupledValue("cj_names", i);
    _cjs_var[i] = coupled("cj_names", i);
  }

  // Resize to number of coupled variables (_nvar from KKSMultiACBulkBase constructor)
  _prop_d2F1dc1darg.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable * cvar = _coupled_moose_vars[i];

    // get second partial derivatives wrt c1 and other coupled variable
    _prop_d2F1dc1darg[i] =
        &getMaterialPropertyDerivative<Real>(_Fj_names[0], _c1_name, cvar->name());
  }
}

Real
KKSMultiACBulkC::computeDFDOP(PFFunctionType type)
{
  Real sum = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_dhjdetai[n])[_qp] * (*_cjs[n])[_qp];

      return -_prop_dF1dc1[_qp] * sum;

    case Jacobian:
      // For when this kernel is used in the Lagrange multiplier equation
      // In that case the Lagrange multiplier is the nonlinear variable
      if (_etai_var != _var.number())
        return 0.0;

      // For when eta_i is the nonlinear variable
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_d2hjdetai2[n])[_qp] * (*_cjs[n])[_qp];

      return -_phi[_j][_qp] * _prop_dF1dc1[_qp] * sum;
  }

  mooseError("Invalid type passed in");
}

Real
KKSMultiACBulkC::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  Real sum = 0.0;
  // Then add dependence of KKSACBulkC on other variables
  // Treat cj variables specially, as they appear in the residual
  if (jvar == _cjs_var[0])
  {
    for (unsigned int n = 0; n < _num_j; ++n)
      sum += (*_prop_dhjdetai[n])[_qp] * (*_cjs[n])[_qp];

    res -= _L[_qp] * (sum * _prop_d2F1dc12[_qp] + _prop_dF1dc1[_qp] * (*_prop_dhjdetai[0])[_qp]) *
           _phi[_j][_qp] * _test[_i][_qp];
    return res;
  }

  for (unsigned int i = 1; i < _num_j; ++i)
  {
    if (jvar == _cjs_var[i])
    {
      res -=
          _L[_qp] * _prop_dF1dc1[_qp] * (*_prop_dhjdetai[i])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
      return res;
    }
  }

  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  for (unsigned int n = 0; n < _num_j; ++n)
    sum += _prop_dF1dc1[_qp] * (*_prop_d2hjdetaidarg[n][cvar])[_qp] * (*_cjs[n])[_qp] +
           (*_prop_d2F1dc1darg[cvar])[_qp] * (*_prop_dhjdetai[n])[_qp] * (*_cjs[n])[_qp];

  res -= _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
