//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
