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

#include "MaterialDerivativeRankFourTestKernel.h"

template <>
InputParameters
validParams<MaterialDerivativeRankFourTestKernel>()
{
  InputParameters params = MaterialDerivativeTestKernelBase<RankFourTensor>::validParams();
  params.addClassDescription(
      "Class used for testing derivatives of a rank four tensor material property.");
  params.addRequiredParam<unsigned int>("i", "Tensor component");
  params.addRequiredParam<unsigned int>("j", "Tensor component");
  params.addRequiredParam<unsigned int>("k", "Tensor component");
  params.addRequiredParam<unsigned int>("l", "Tensor component");
  return params;
}

MaterialDerivativeRankFourTestKernel::MaterialDerivativeRankFourTestKernel(
    const InputParameters & parameters)
  : MaterialDerivativeTestKernelBase<RankFourTensor>(parameters),
    _component_i(getParam<unsigned int>("i")),
    _component_j(getParam<unsigned int>("j")),
    _component_k(getParam<unsigned int>("k")),
    _component_l(getParam<unsigned int>("l"))
{
}

Real
MaterialDerivativeRankFourTestKernel::computeQpResidual()
{
  return _p[_qp](_component_i, _component_j, _component_k, _component_l) * _test[_i][_qp];
}

Real
MaterialDerivativeRankFourTestKernel::computeQpJacobian()
{
  return _p_diag_derivative[_qp](_component_i, _component_j, _component_k, _component_l) *
         _phi[_j][_qp] * _test[_i][_qp];
}

Real
MaterialDerivativeRankFourTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable number corresponding to jvar
  const unsigned int cvar = mapJvarToCvar(jvar);
  return (*_p_off_diag_derivatives[cvar])[_qp](
             _component_i, _component_j, _component_k, _component_l) *
         _phi[_j][_qp] * _test[_i][_qp];
}
