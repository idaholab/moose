/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MaterialDerivativeRankTwoTestKernel.h"

template <>
InputParameters
validParams<MaterialDerivativeRankTwoTestKernel>()
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
