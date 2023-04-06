//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonFunctorSideDiffusiveFluxIntegral.h"
#include "MathFVUtils.h"

#include "metaphysicl/raw_type.h"

using namespace Moose;
using namespace FV;

registerMooseObject("MooseTestApp", NonFunctorSideDiffusiveFluxIntegral);
registerMooseObject("MooseTestApp", ADNonFunctorSideDiffusiveFluxIntegral);

template <bool is_ad>
InputParameters
NonFunctorSideDiffusiveFluxIntegralTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable which this postprocessor integrates");
  params.addParam<MaterialPropertyName>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation. "
      "This must be provided if the variable is of finite element type");
  return params;
}

template <bool is_ad>
NonFunctorSideDiffusiveFluxIntegralTempl<is_ad>::NonFunctorSideDiffusiveFluxIntegralTempl(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _fv(_fv_variable),
    _diffusion_coef(getGenericMaterialProperty<Real, is_ad>("diffusivity"))
{
  addMooseVariableDependency(&mooseVariableField());
}

template <bool is_ad>
Real
NonFunctorSideDiffusiveFluxIntegralTempl<is_ad>::computeQpIntegral()
{
  if (_fv)
  {
    // Get the face info
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");

    // Get the gradient of the variable on the face
    const auto & grad_u =
        MetaPhysicL::raw_value(_fv_variable->adGradSln(*fi, Moose::currentState()));

    // FIXME Get the diffusion coefficient on the face, see #16809
    return -diffusivityGradientProduct(grad_u, MetaPhysicL::raw_value(_diffusion_coef[_qp])) *
           _normals[_qp];
  }
  else
    return -diffusivityGradientProduct(_grad_u[_qp], MetaPhysicL::raw_value(_diffusion_coef[_qp])) *
           _normals[_qp];
}

template <bool is_ad>
RealVectorValue
NonFunctorSideDiffusiveFluxIntegralTempl<is_ad>::diffusivityGradientProduct(
    const RealVectorValue & grad_u, const Real diffusivity)
{
  return grad_u * diffusivity;
}

template class NonFunctorSideDiffusiveFluxIntegralTempl<false>;
template class NonFunctorSideDiffusiveFluxIntegralTempl<true>;
