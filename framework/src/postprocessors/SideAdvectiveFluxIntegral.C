//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideAdvectiveFluxIntegral.h"
#include "MathFVUtils.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideAdvectiveFluxIntegral);
registerMooseObject("MooseApp", ADSideAdvectiveFluxIntegral);

template <bool is_ad, typename T>
InputParameters
SideAdvectiveFluxIntegralTempl<is_ad, T>::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  MooseEnum component("x y z normal");

  params.addRequiredParam<MooseEnum>("component", component, "The desired component of flux.");
  params.addRequiredParam<MooseFunctorName>("vel_x", "x-component of the velocity vector");
  params.addParam<MooseFunctorName>("vel_y", "y-component of the velocity vector");
  params.addParam<MooseFunctorName>("vel_z", "z-component of the velocity vector");
  params.addParam<MooseFunctorName>(
      "advected_variable",
      0,
      "The advected variable quantity of which to study the flow; useful for "
      "finite element simulations");
  params.addParam<MaterialPropertyName>(
      "advected_mat_prop",
      0,
      "The advected material property of which to study the flow; "
      "useful for finite element simulations");
  params.addParam<MooseFunctorName>("advected_quantity",
                                    "The quantity to advect. This is the canonical parameter to "
                                    "set the advected quantity when finite volume is being used.");

  params.addClassDescription("Computes the volumetric advected quantity through a sideset.");

  return params;
}

template <bool is_ad, typename T>
SideAdvectiveFluxIntegralTempl<is_ad, T>::SideAdvectiveFluxIntegralTempl(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _use_normal(getParam<MooseEnum>("component") == "normal"),
    _component(getParam<MooseEnum>("component")),
    _advected_variable_supplied(parameters.isParamSetByUser("advected_variable")),
    _advected_variable(getFunctor<Real>("advected_variable")),
    _advected_mat_prop_supplied(parameters.isParamSetByUser("advected_mat_prop")),
    _advected_material_property(getGenericMaterialProperty<T, is_ad>("advected_mat_prop")),
    _adv_quant(isParamValid("advected_quantity") ? &getFunctor<Real>("advected_quantity")
                                                 : nullptr),
    _vel_x(getFunctor<Real>("vel_x")),
    _vel_y(_mesh.dimension() >= 2 ? &getFunctor<Real>("vel_y") : nullptr),
    _vel_z(_mesh.dimension() == 3 ? &getFunctor<Real>("vel_z") : nullptr)
{
  // Check that at most one advected quantity has been provided
  if (_advected_variable_supplied && _advected_mat_prop_supplied)
    mooseError(
        "SideAdvectiveFluxIntegralPostprocessor should be provided either an advected variable "
        "or an advected material property");
  // Check whether a finite element or finite volume variable is provide
  _qp_integration = !_adv_quant;
}

template <bool is_ad, typename T>
Real
SideAdvectiveFluxIntegralTempl<is_ad, T>::computeFaceInfoIntegral(const FaceInfo * const fi)
{
  using MetaPhysicL::raw_value;

  mooseAssert(fi, "We should have a face info in " + name());
  mooseAssert(_adv_quant, "We should have an advected quantity in " + name());

  const auto state = determineState();

  const auto adv_quant_face = raw_value((*_adv_quant)(
      Moose::FaceArg({fi, Moose::FV::LimiterType::Upwind, true, false, nullptr}), state));

  // Get face value for velocity
  const auto vel_x = raw_value(
      (_vel_x)(Moose::FaceArg({fi, Moose::FV::LimiterType::Upwind, true, false, nullptr}), state));
  const auto vel_y =
      _vel_y
          ? raw_value((*_vel_y)(
                Moose::FaceArg({fi, Moose::FV::LimiterType::Upwind, true, false, nullptr}), state))
          : 0;
  const auto vel_z =
      _vel_z
          ? raw_value((*_vel_z)(
                Moose::FaceArg({fi, Moose::FV::LimiterType::Upwind, true, false, nullptr}), state))
          : 0;

  return fi->normal() * adv_quant_face * RealVectorValue(vel_x, vel_y, vel_z);
}

template <bool is_ad, typename T>
Real
SideAdvectiveFluxIntegralTempl<is_ad, T>::computeQpIntegral()
{
  using MetaPhysicL::raw_value;

  const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
  const auto state = determineState();
  const auto vel_x = raw_value(_vel_x(qp_arg, state));
  const auto vel_y = _vel_y ? raw_value((*_vel_y)(qp_arg, state)) : 0;
  const auto vel_z = _vel_z ? raw_value((*_vel_z)(qp_arg, state)) : 0;
  const Moose::ElemSideQpArg side_arg = {_current_elem, _current_side, _qp, _qrule};

  if (_advected_variable_supplied)
    return (_use_normal ? raw_value(_advected_variable(side_arg, state)) *
                              RealVectorValue(vel_x, vel_y, vel_z) * _normals[_qp]
                        : raw_value(_advected_variable(qp_arg, state)) *
                              RealVectorValue(vel_x, vel_y, vel_z)(_component));
  else if (_advected_mat_prop_supplied)
    return (_use_normal ? raw_value(_advected_material_property[_qp]) *
                              RealVectorValue(vel_x, vel_y, vel_z) * _normals[_qp]
                        : raw_value(_advected_material_property[_qp]) *
                              RealVectorValue(vel_x, vel_y, vel_z)(_component));
  else
    return (_use_normal ? RealVectorValue(vel_x, vel_y, vel_z) * _normals[_qp]
                        : RealVectorValue(vel_x, vel_y, vel_z)(_component));
}

template class SideAdvectiveFluxIntegralTempl<false, Real>;
template class SideAdvectiveFluxIntegralTempl<true, Real>;
