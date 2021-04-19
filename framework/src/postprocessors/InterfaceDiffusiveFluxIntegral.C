//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceDiffusiveFluxIntegral.h"
#include "FVUtils.h"

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
  params.addCoupledVar(
      "neighbor_variable",
      "The name of the variable on the secondary side of the interface. By default, "
      "the primary side variable name is used for the secondary side");
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity", "The name of the diffusivity property on the primary side of the interface");
  params.addParam<MaterialPropertyName>("neighbor_diffusivity",
                                        "The name of the diffusivity property on the secondary "
                                        "side of the interface. By default, the "
                                        "primary side material property name is used for the "
                                        "secondary side. Only needed for finite volume");
  params.addClassDescription("Computes the diffusive flux on the interface.");
  params.set<unsigned short>("ghost_layers") = 2;

  return params;
}

template <bool is_ad>
InterfaceDiffusiveFluxIntegralTempl<is_ad>::InterfaceDiffusiveFluxIntegralTempl(
    const InputParameters & parameters)
  : InterfaceIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _grad_u(coupledGradient("variable")),
    _neighbor_fv_variable(
        parameters.isParamSetByUser("neighbor_variable")
            ? dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("neighbor_variable", 0))
            : dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("variable", 0))),
    _fv(_fv_variable),
    _diffusion_coef(
        getGenericMaterialProperty<Real, is_ad>(getParam<MaterialPropertyName>("diffusivity"))),
    _diffusion_coef_neighbor(parameters.isParamSetByUser("neighbor_diffusivity")
                                 ? getGenericNeighborMaterialProperty<Real, is_ad>(
                                       getParam<MaterialPropertyName>("neighbor_diffusivity"))
                                 : getGenericNeighborMaterialProperty<Real, is_ad>(
                                       getParam<MaterialPropertyName>("diffusivity")))
{
  addMooseVariableDependency(&mooseVariableField());

  // Primary and secondary variable should both be of a similar variable type
  if (parameters.isParamSetByUser("neighbor_variable"))
    if ((_fv && !getFieldVar("neighbor_variable", 0)->isFV()) ||
        (!_fv && getFieldVar("neighbor_variable", 0)->isFV()))
      mooseError("For the InterfaceDiffusiveFluxIntegral, variable and "
                 "neighbor_variable should be of a similar variable type.");

  // Warn that we are not using the same gradient to compute the diffusive flux here
  if (_fv && (_fv_variable == _neighbor_fv_variable))
    mooseWarning("Only one finite volume variable was specified. InterfaceDiffusiveFluxIntegral is "
                 "only accurate at a "
                 "FVDiffusionInterface, not with a regular diffusion kernel.");
}

template <bool is_ad>
Real
InterfaceDiffusiveFluxIntegralTempl<is_ad>::computeQpIntegral()
{
  if (_fv)
  {
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");
    const auto normal = fi->normal();

    // Form a finite difference gradient across the interface
    Point du = _current_elem->centroid() - _neighbor_elem->centroid();
    du /= (du * du);
    const auto gradient = (_fv_variable->getElemValue(_current_elem) -
                           _neighbor_fv_variable->getElemValue(_neighbor_elem)) *
                          du;

    Real diffusivity;
    interpolate(Moose::FV::InterpMethod::Average,
                diffusivity,
                MetaPhysicL::raw_value(_diffusion_coef[_qp]),
                MetaPhysicL::raw_value(_diffusion_coef_neighbor[_qp]),
                *fi,
                true);

    return -diffusivity * MetaPhysicL::raw_value(gradient * normal);
  }
  else
    return -MetaPhysicL::raw_value(_diffusion_coef[_qp]) * _grad_u[_qp] * _normals[_qp];
}

template class InterfaceDiffusiveFluxIntegralTempl<false>;
template class InterfaceDiffusiveFluxIntegralTempl<true>;
