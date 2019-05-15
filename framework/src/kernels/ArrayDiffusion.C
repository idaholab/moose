//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayDiffusion.h"

registerMooseObject("MooseApp", ArrayDiffusion);

template <>
InputParameters
validParams<ArrayDiffusion>()
{
  InputParameters params = validParams<ArrayKernel>();
  params.addParam<MaterialPropertyName>("diffusion_coefficient", "The name of the diffusivity");
  MooseEnum opt("scalar=0 array=1 full=2", "array");
  params.addParam<MooseEnum>("diffusion_coefficient_type", opt, "Diffusion coefficient type");
  params.addClassDescription(
      "The array Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak "
      "form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

ArrayDiffusion::ArrayDiffusion(const InputParameters & parameters)
  : ArrayKernel(parameters), _dc_type(getParam<MooseEnum>("diffusion_coefficient_type"))
{
  if (_dc_type == 0)
    _d = &getMaterialProperty<Real>("diffusion_coefficient");
  else if (_dc_type == 1)
    _d_array = &getMaterialProperty<RealEigenVector>("diffusion_coefficient");
  else if (_dc_type == 2)
    _d_2d_array = &getMaterialProperty<RealArray>("diffusion_coefficient");
}

RealEigenVector
ArrayDiffusion::computeQpResidual()
{
  if (_dc_type == 0)
    return _grad_u[_qp] * _array_grad_test[_i][_qp] * (*_d)[_qp];

  else if (_dc_type == 1)
  {
    mooseAssert((*_d_array)[_qp].size() == _var.count(),
                "diffusion_coefficient size is inconsistent with the number of components of array "
                "variable");
    mooseAssert((*_d_array)[_qp].size() == _var.count(), "");
    RealEigenVector v = _grad_u[_qp] * _array_grad_test[_i][_qp];
    for (unsigned int i = 0; i < _var.count(); ++i)
      v(i) *= (*_d_array)[_qp](i);
    return v;
  }

  else
  {
    mooseAssert((*_d_2d_array)[_qp].cols() == _var.count(),
                "diffusion_coefficient size is inconsistent with the number of components of array "
                "variable");
    mooseAssert((*_d_2d_array)[_qp].rows() == _var.count(),
                "diffusion_coefficient size is inconsistent with the number of components of array "
                "variable");
    return (*_d_2d_array)[_qp] * (_grad_u[_qp] * _array_grad_test[_i][_qp]);
  }
}

RealEigenVector
ArrayDiffusion::computeQpJacobian()
{
  if (_dc_type == 0)
    return RealEigenVector::Constant(_var.count(),
                                     _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d)[_qp]);
  else if (_dc_type == 1)
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_array)[_qp];
  else
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_2d_array)[_qp].diagonal();
}

RealArray
ArrayDiffusion::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number() && _dc_type == 2)
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_2d_array)[_qp];
  else
    return ArrayKernel::computeQpOffDiagJacobian(jvar);
}
