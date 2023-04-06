//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideDiffusiveFluxIntegral.h"
#include "MathFVUtils.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideDiffusiveFluxIntegral);
registerMooseObject("MooseApp", ADSideDiffusiveFluxIntegral);
registerMooseObject("MooseApp", SideVectorDiffusivityFluxIntegral);
registerMooseObject("MooseApp", ADSideVectorDiffusivityFluxIntegral);
registerMooseObjectRenamed("MooseApp",
                           SideFluxIntegral,
                           "06/30/2021 24:00",
                           SideDiffusiveFluxIntegral);
registerMooseObjectRenamed("MooseApp",
                           ADSideFluxIntegral,
                           "06/30/2021 24:00",
                           ADSideDiffusiveFluxIntegral);

template <bool is_ad, typename T>
InputParameters
SideDiffusiveFluxIntegralTempl<is_ad, T>::validParams()
{
  InputParameters params = SideIntegralVariablePostprocessor::validParams();
  params.addParam<MaterialPropertyName>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation. "
      "This must be provided if the variable is of finite element type");
  params.addParam<MooseFunctorName>(
      "functor_diffusivity",
      "The name of the diffusivity functor that will be used in the flux computation. This must be "
      "provided if the variable is of finite volume type");
  params.addClassDescription(
      "Computes the integral of the diffusive flux over the specified boundary");
  return params;
}

template <bool is_ad, typename T>
SideDiffusiveFluxIntegralTempl<is_ad, T>::SideDiffusiveFluxIntegralTempl(
    const InputParameters & parameters)
  : SideIntegralVariablePostprocessor(parameters),
    _diffusion_coef(isParamValid("diffusivity")
                        ? &getGenericMaterialProperty<T, is_ad>("diffusivity")
                        : nullptr),
    _functor_diffusion_coef(isParamValid("functor_diffusivity")
                                ? &getFunctor<Moose::GenericType<T, is_ad>>("functor_diffusivity")
                                : nullptr)
{
  if (_fv && !isParamValid("functor_diffusivity"))
    mooseError(
        "For a finite volume variable, the parameter 'functor_diffusivity' must be provided");
  if (!_fv && !isParamValid("diffusivity"))
    mooseError("For a finite element variable, the parameter 'diffusivity' must be provided");
}

template <bool is_ad, typename T>
Real
SideDiffusiveFluxIntegralTempl<is_ad, T>::computeFaceInfoIntegral(const FaceInfo * const fi)
{
  // Get the gradient of the variable on the face
  const auto grad_u =
      MetaPhysicL::raw_value(_fv_variable->gradient(makeCDFace(*fi), determineState()));

  return -diffusivityGradientProduct(grad_u,
                                     MetaPhysicL::raw_value((*_functor_diffusion_coef)(
                                         makeCDFace(*fi), determineState()))) *
         _normals[_qp];
}

template <bool is_ad, typename T>
Real
SideDiffusiveFluxIntegralTempl<is_ad, T>::computeQpIntegral()
{
  return -diffusivityGradientProduct(_grad_u[_qp],
                                     MetaPhysicL::raw_value((*_diffusion_coef)[_qp])) *
         _normals[_qp];
}

template <bool is_ad, typename T>
RealVectorValue
SideDiffusiveFluxIntegralTempl<is_ad, T>::diffusivityGradientProduct(const RealVectorValue & grad_u,
                                                                     const Real diffusivity)
{
  return grad_u * diffusivity;
}

template <bool is_ad, typename T>
RealVectorValue
SideDiffusiveFluxIntegralTempl<is_ad, T>::diffusivityGradientProduct(
    const RealVectorValue & grad_u, const RealVectorValue & diffusivity)
{
  RealVectorValue d_grad_u = grad_u;
  for (const auto i : make_range(Moose::dim))
    d_grad_u(i) *= diffusivity(i);

  return d_grad_u;
}

template class SideDiffusiveFluxIntegralTempl<false, Real>;
template class SideDiffusiveFluxIntegralTempl<true, Real>;
template class SideDiffusiveFluxIntegralTempl<false, RealVectorValue>;
template class SideDiffusiveFluxIntegralTempl<true, RealVectorValue>;
