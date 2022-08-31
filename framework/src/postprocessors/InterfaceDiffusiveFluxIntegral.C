//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceDiffusiveFluxIntegral.h"
#include "MathFVUtils.h"
#include "FaceInfo.h"

#include "libmesh/remote_elem.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", InterfaceDiffusiveFluxIntegral);
registerMooseObject("MooseApp", ADInterfaceDiffusiveFluxIntegral);

template <bool is_ad>
InputParameters
InterfaceDiffusiveFluxIntegralTempl<is_ad>::validParams()
{
  InputParameters params = InterfaceIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("variable",
                               "The name of the variable on the primary side of the interface");
  params.addCoupledVar("neighbor_variable",
                       "The name of the variable on the secondary side of the interface.");
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity", "The name of the diffusivity property on the primary side of the interface");
  params.addParam<MaterialPropertyName>("neighbor_diffusivity",
                                        "The name of the diffusivity property on the secondary "
                                        "side of the interface. By default, the "
                                        "primary side material property name is used for the "
                                        "secondary side. Only needed for finite volume");
  MooseEnum interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");
  params.addClassDescription("Computes the diffusive flux on the interface.");

  return params;
}

template <bool is_ad>
InterfaceDiffusiveFluxIntegralTempl<is_ad>::InterfaceDiffusiveFluxIntegralTempl(
    const InputParameters & parameters)
  : InterfaceIntegralPostprocessor(parameters),
    _grad_u(coupledGradient("variable")),
    _u(coupledValue("variable")),
    _u_neighbor(parameters.isParamSetByUser("neighbor_variable")
                    ? coupledNeighborValue("neighbor_variable")
                    : coupledNeighborValue("variable")),
    _diffusion_coef(
        getGenericMaterialProperty<Real, is_ad>(getParam<MaterialPropertyName>("diffusivity"))),
    _diffusion_coef_neighbor(parameters.isParamSetByUser("neighbor_diffusivity")
                                 ? getGenericNeighborMaterialProperty<Real, is_ad>(
                                       getParam<MaterialPropertyName>("neighbor_diffusivity"))
                                 : getGenericNeighborMaterialProperty<Real, is_ad>(
                                       getParam<MaterialPropertyName>("diffusivity")))
{

  // Primary and secondary variable should both be of a similar variable type
  if (parameters.isParamSetByUser("neighbor_variable"))
    if ((_has_fv_vars && !getFieldVar("neighbor_variable", 0)->isFV()) ||
        (!_has_fv_vars && getFieldVar("neighbor_variable", 0)->isFV()))
      mooseError("For the InterfaceDiffusiveFluxIntegral, variable and "
                 "neighbor_variable should be of a similar variable type.");

  if (!_has_fv_vars && parameters.isParamSetByUser("coeff_interp_method"))
    paramError(
        "coeff_interp_method",
        "This parameter should not be defined for the postprocessing of finite element variables!");

  const auto & interp_method = getParam<MooseEnum>("coeff_interp_method");
  if (interp_method == "average")
    _coeff_interp_method = Moose::FV::InterpMethod::Average;
  else if (interp_method == "harmonic")
    _coeff_interp_method = Moose::FV::InterpMethod::HarmonicAverage;
}

template <bool is_ad>
Real
InterfaceDiffusiveFluxIntegralTempl<is_ad>::computeQpIntegral()
{
  if (_has_fv_vars)
  {
    mooseAssert(_fi, "This should never be null. If it is then something went wrong in execute()");

    const auto normal = _fi->normal();

    // Form a finite difference gradient across the interface
    Point one_over_gradient_support = _fi->elemCentroid() - _fi->neighborCentroid();
    one_over_gradient_support /= (one_over_gradient_support * one_over_gradient_support);
    const auto gradient = (_u[_qp] - _u_neighbor[_qp]) * one_over_gradient_support;

    Real diffusivity;
    interpolate(_coeff_interp_method,
                diffusivity,
                MetaPhysicL::raw_value(_diffusion_coef[_qp]),
                MetaPhysicL::raw_value(_diffusion_coef_neighbor[_qp]),
                *_fi,
                true);

    return -diffusivity * MetaPhysicL::raw_value(gradient * normal);
  }
  else
    return -MetaPhysicL::raw_value(_diffusion_coef[_qp]) * _grad_u[_qp] * _normals[_qp];
}

template class InterfaceDiffusiveFluxIntegralTempl<false>;
template class InterfaceDiffusiveFluxIntegralTempl<true>;
