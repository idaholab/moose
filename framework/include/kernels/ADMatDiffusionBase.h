//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelGrad.h"

/**
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotropic diffusion or RealTensorValue for the general anisotropic case.
 */
template <typename T>
class ADMatDiffusionBase : public ADKernelGrad
{
public:
  static InputParameters validParams();

  ADMatDiffusionBase(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// diffusion coefficient
  const ADMaterialProperty<T> & _diffusivity;

  /// Gradient of the concentration
  const ADVariableGradient & _grad_v;
};

template <typename T>
ADMatDiffusionBase<T>::ADMatDiffusionBase(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _diffusivity(getADMaterialProperty<T>("diffusivity")),
    _grad_v(isCoupled("v") ? adCoupledGradient("v") : _grad_u)
{
}

template <typename T>
ADRealVectorValue
ADMatDiffusionBase<T>::precomputeQpResidual()
{
  return _diffusivity[_qp] * _grad_v[_qp];
}

template <typename T>
InputParameters
ADMatDiffusionBase<T>::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("Diffusion kernel with a material property as diffusivity and "
                             "automatic differentiation to provide perfect Jacobians");
  params.addParam<MaterialPropertyName>(
      "diffusivity", "D", "The diffusivity value or material property");
  params.addCoupledVar("v",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  return params;
}
