//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSMultiACBulkC.h"

registerMooseObject("PhaseFieldApp", KKSMultiACBulkC);

InputParameters
KKSMultiACBulkC::validParams()
{
  InputParameters params = KKSMultiACBulkBase::validParams();
  params.addClassDescription("Multi-phase KKS model kernel (part 2 of 2) for the Bulk Allen-Cahn. "
                             "This includes all terms dependent on chemical potential.");
  params.addRequiredCoupledVar(
      "cj_names", "Array of phase concentrations cj. Place in same order as Fj_names!");
  return params;
}

KKSMultiACBulkC::KKSMultiACBulkC(const InputParameters & parameters)
  : KKSMultiACBulkBase(parameters),
    // Can use any dFj/dcj since they are equal so pick first cj in the list
    _c1_name(coupledName("cj_names", 0)),
    _cjs(coupledValues("cj_names")),
    _cjs_var(coupledIndices("cj_names")),
    _prop_dF1dc1(getMaterialPropertyDerivative<Real>(_Fj_names[0],
                                                     _c1_name)), // Use first Fj in list for dFj/dcj
    _prop_d2F1dc12(getMaterialPropertyDerivative<Real>(_Fj_names[0], _c1_name, _c1_name)),
    _prop_d2F1dc1darg(_n_args)
{
  if (_num_j != coupledComponents("cj_names"))
    paramError("cj_names", "Need to pass in as many cj_names as Fj_names");

  // get second partial derivatives wrt c1 and other coupled variable
  for (unsigned int i = 0; i < _n_args; ++i)
    _prop_d2F1dc1darg[i] = &getMaterialPropertyDerivative<Real>(_Fj_names[0], _c1_name, i);
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
