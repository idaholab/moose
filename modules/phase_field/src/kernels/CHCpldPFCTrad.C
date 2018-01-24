/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CHCpldPFCTrad.h"

template <>
InputParameters
validParams<CHCpldPFCTrad>()
{
  InputParameters params = validParams<LaplacianSplit>();
  params.addRequiredParam<MaterialPropertyName>("coeff_name", "Name of coefficient");
  return params;
}

CHCpldPFCTrad::CHCpldPFCTrad(const InputParameters & parameters)
  : LaplacianSplit(parameters), _coeff(getMaterialProperty<Real>("coeff_name"))
{
}

RealGradient
CHCpldPFCTrad::precomputeQpResidual()
{
  RealGradient grad_cpldvar = LaplacianSplit::precomputeQpResidual();
  return _coeff[_qp] * grad_cpldvar;
}

Real
CHCpldPFCTrad::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real grphi_grtst = LaplacianSplit::computeQpOffDiagJacobian(jvar);
  return _coeff[_qp] * grphi_grtst;
}
