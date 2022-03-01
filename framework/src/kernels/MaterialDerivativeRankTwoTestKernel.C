//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialDerivativeRankTwoTestKernel.h"

registerMooseObject("MooseApp", MaterialDerivativeRankTwoTestKernel);

InputParameters
MaterialDerivativeRankTwoTestKernel::validParams()
{
  InputParameters params = MaterialDerivativeTestKernelBase<RankTwoTensor>::validParams();
  params.addClassDescription(
      "Class used for testing derivatives of a rank two tensor material property.");
  params.addRequiredParam<unsigned int>("i", "Tensor component");
  params.addRequiredParam<unsigned int>("j", "Tensor component");
  return params;
}

MaterialDerivativeRankTwoTestKernel::MaterialDerivativeRankTwoTestKernel(
    const InputParameters & parameters)
  : MaterialDerivativeTestKernelBase<RankTwoTensor>(parameters),
    _component_i(getParam<unsigned int>("i")),
    _component_j(getParam<unsigned int>("j"))
{
}

Real
MaterialDerivativeRankTwoTestKernel::computeQpResidual()
{
  return _p[_qp](_component_i, _component_j) * _test[_i][_qp];
}

Real
MaterialDerivativeRankTwoTestKernel::computeQpJacobian()
{
  return _p_diag_derivative[_qp](_component_i, _component_j) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
MaterialDerivativeRankTwoTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable number corresponding to jvar
  const unsigned int cvar = mapJvarToCvar(jvar);
  return (*_p_off_diag_derivatives[cvar])[_qp](_component_i, _component_j) * _phi[_j][_qp] *
         _test[_i][_qp];
}
