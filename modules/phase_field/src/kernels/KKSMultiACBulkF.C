//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSMultiACBulkF.h"

registerMooseObject("PhaseFieldApp", KKSMultiACBulkF);

InputParameters
KKSMultiACBulkF::validParams()
{
  InputParameters params = KKSMultiACBulkBase::validParams();
  params.addClassDescription("KKS model kernel (part 1 of 2) for the Bulk Allen-Cahn. This "
                             "includes all terms NOT dependent on chemical potential.");
  params.addRequiredParam<Real>("wi", "Double well height parameter");
  params.addRequiredParam<MaterialPropertyName>(
      "gi_name", "Base name for the double well function g_i(eta_i)");
  return params;
}

KKSMultiACBulkF::KKSMultiACBulkF(const InputParameters & parameters)
  : KKSMultiACBulkBase(parameters),
    _wi(getParam<Real>("wi")),
    _prop_dgi(getMaterialPropertyDerivative<Real>("gi_name", _etai_name)),
    _prop_d2gi(getMaterialPropertyDerivative<Real>("gi_name", _etai_name, _etai_name))
{
}

Real
KKSMultiACBulkF::computeDFDOP(PFFunctionType type)
{
  Real sum = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];

      return sum + _wi * _prop_dgi[_qp];

    case Jacobian:
      // For when this kernel is used in the Lagrange multiplier equation
      // In that case the Lagrange multiplier is the nonlinear variable
      if (_etai_var != _var.number())
        return 0.0;

      // For when eta_i is the nonlinear variable
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_d2hjdetai2[n])[_qp] * (*_prop_Fj[n])[_qp];

      return _phi[_j][_qp] * (sum + _wi * _prop_d2gi[_qp]);
  }

  mooseError("Invalid type passed in");
}

Real
KKSMultiACBulkF::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  // Then add dependence of KKSMultiACBulkF on other variables
  Real sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_d2hjdetaidarg[n][cvar])[_qp] * (*_prop_Fj[n])[_qp] +
           (*_prop_dhjdetai[n])[_qp] * (*_prop_dFjdarg[n][cvar])[_qp];

  // Handle the case when this kernel is used in the Lagrange multiplier equation
  // In this case the second derivative of the barrier function contributes
  // to the off-diagonal Jacobian
  if (jvar == _etai_var)
    sum += _wi * _prop_d2gi[_qp];

  res += _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
