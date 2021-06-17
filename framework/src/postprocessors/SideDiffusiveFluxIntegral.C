//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideDiffusiveFluxIntegral.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideDiffusiveFluxIntegral);
registerMooseObject("MooseApp", ADSideDiffusiveFluxIntegral);
registerMooseObjectRenamed("MooseApp",
                           SideFluxIntegral,
                           "06/30/2021 24:00",
                           SideDiffusiveFluxIntegral);
registerMooseObjectRenamed("MooseApp",
                           ADSideFluxIntegral,
                           "06/30/2021 24:00",
                           ADSideDiffusiveFluxIntegral);

defineLegacyParams(SideDiffusiveFluxIntegral);

template <bool is_ad, T material_type>
InputParameters
SideDiffusiveFluxIntegralTempl<is_ad, material_type>::validParams()
{
  InputParameters params = SideIntegralVariablePostprocessor::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation.");
  params.addClassDescription(
      "Computes the integral of the diffusive flux over the specified boundary");
  return params;
}

template <bool is_ad, T material_type>
SideDiffusiveFluxIntegralTempl<is_ad, material_type>::SideDiffusiveFluxIntegralTempl(
    const InputParameters & parameters)
  : SideIntegralVariablePostprocessor(parameters),
    _diffusivity(getParam<MaterialPropertyName>("diffusivity")),
    _diffusion_coef(getGenericMaterialProperty<material_type, is_ad>(_diffusivity))
{
}

template <bool is_ad, T material_type>
Real
SideDiffusiveFluxIntegralTempl<is_ad, material_type>::computeQpIntegral()
{
  if (_fv)
  {
    // Get the face info
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");

    // Get the gradient of the variable on the face
    const auto & grad_u = MetaPhysicL::raw_value(_fv_variable->adGradSln(*fi));

    // FIXME Get the diffusion coefficient on the face, see #16809
    return -diffusivity_gradient_product(
        MetaPhysicL::raw_value(_diffusion_coef[_qp]), grad_u) * _normals[_qp];
  }
  else
    return -diffusivity_gradient_product(
        MetaPhysicL::raw_value(_diffusion_coef[_qp]), _grad_u[_qp]) * _normals[_qp];
}

template <bool is_ad, T material_type>
RealVectorValue
SideDiffusiveFluxIntegralTempl<is_ad, material_type>::diffusivity_gradient_product(
    RealVectorValue grad_u,
    material_type diffusivity)
{

  return grad_u * diffusivity;
}

template class SideDiffusiveFluxIntegralTempl<false, Real>;
template class SideDiffusiveFluxIntegralTempl<true, Real>;
template class SideDiffusiveFluxIntegralTempl<false, RealVectorValue>;
template class SideDiffusiveFluxIntegralTempl<true, RealVectorValue>;
