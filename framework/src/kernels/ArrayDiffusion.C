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
  params.addClassDescription("The array Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak "
                             "form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

ArrayDiffusion::ArrayDiffusion(const InputParameters & parameters) :
    ArrayKernel(parameters),
    _dc_type(getParam<MooseEnum>("diffusion_coefficient_type"))
{
  if (_dc_type == 0)
    _d = &getMaterialProperty<Real>("diffusion_coefficient");
  else if (_dc_type == 1)
    _d_array = &getMaterialProperty<RealArrayValue>("diffusion_coefficient");
  else if (_dc_type == 2)
    _d_2d_array = &getMaterialProperty<RealArray>("diffusion_coefficient");
}

RealArrayValue
ArrayDiffusion::computeQpResidual()
{
  if (_dc_type == 0)
    return _grad_u[_qp] * _array_grad_test[_i][_qp] * (*_d)[_qp];

  else if (_dc_type == 1)
  {
    mooseAssert((*_d_array)[_qp].size() == _var.count(), "");
    RealArrayValue v = _grad_u[_qp] * _array_grad_test[_i][_qp];
    for (unsigned int i = 0; i < _var.count(); ++i)
      v(i) *= (*_d_array)[_qp](i);
    return v;
  }

  else
  {
    mooseAssert((*_d_2d_array)[_qp].cols() == _var.count(), "");
    mooseAssert((*_d_2d_array)[_qp].rows() == _var.count(), "");
    return (*_d_2d_array)[_qp] * (_grad_u[_qp] * _array_grad_test[_i][_qp]);
  }
}

RealArrayValue
ArrayDiffusion::computeQpJacobian()
{
  if (_dc_type == 0)
    return RealArrayValue::Constant(_var.count(), _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d)[_qp]);
  else if (_dc_type == 1)
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_array)[_qp];
  else
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_2d_array)[_qp].diagonal();
}

RealArray
ArrayDiffusion::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
  {
    if (_dc_type == 0 || _dc_type == 1)
    {
      RealArrayValue v = computeQpJacobian();
      RealArray t(_var.count(), _var.count());
      t.diagonal() = v;
      return t;
    }
    else
      return _grad_phi[_j][_qp] * _grad_test[_i][_qp] * (*_d_2d_array)[_qp];
  }
  else
    return RealArray(_var.count(), jvar.count());
}
