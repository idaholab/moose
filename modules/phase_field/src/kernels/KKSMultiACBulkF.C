/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSMultiACBulkF.h"

template<>
InputParameters validParams<KKSMultiACBulkF>()
{
  InputParameters params = validParams<KKSMultiACBulkBase>();
  params.addClassDescription("KKS model kernel (part 1 of 2) for the Bulk Allen-Cahn. This includes all terms NOT dependent on chemical potential.");
  params.addRequiredParam<Real>("wi", "Double well height parameter");
  params.addRequiredParam<MaterialPropertyName>("gi_name", "Base name for the double well function g_i(eta)");
  return params;
}

KKSMultiACBulkF::KKSMultiACBulkF(const InputParameters & parameters) :
    KKSMultiACBulkBase(parameters),
    _wi(getParam<Real>("wi")),
    _prop_dgi(getMaterialPropertyDerivative<Real>("gi_name", _etai_name)),
    _prop_d2gi(getMaterialPropertyDerivative<Real>("gi_name", _etai_name, _etai_name))
{
}

Real
KKSMultiACBulkF::computeDFDOP(PFFunctionType type)
{
  Real res = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int n = 0; n < _num_Fj; ++n)
        res += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];

      return res + _wi * _prop_dgi[_qp];

    case Jacobian:
    {
      for (unsigned int n = 0; n < _num_Fj; ++n)
        res += (*_prop_d2hjdetai2[n])[_qp] * (*_prop_Fj[n])[_qp];

      return _phi[_j][_qp] * res + _wi * _prop_d2gi[_qp];
    }
  }

  mooseError("Invalid type passed in");
}

Real
KKSMultiACBulkF::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  // Then add dependence of KKSMultiACBulkF on other variables
  Real sum = 0.0;
  for (unsigned int n = 0; n < _num_Fj; ++n)
    sum += (*_prop_d2hjdetaidarg[n][cvar])[_qp] * (*_prop_Fj[n])[_qp]
            + (*_prop_dhjdetai[n])[_qp] * (*_prop_dFjdarg[n][cvar])[_qp];

  res += _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

  return res;
}
