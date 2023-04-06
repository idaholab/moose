//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorElementalGradientAux.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", FunctorElementalGradientAux);
registerMooseObject("MooseApp", ADFunctorElementalGradientAux);

template <bool is_ad>
InputParameters
FunctorElementalGradientAuxTempl<is_ad>::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription(
      "Evaluates the gradient of a functor (variable, function or functor material property) on "
      "the current element or quadrature point.");
  params.addRequiredParam<MooseFunctorName>("functor", "The functor to evaluate");
  params.addParam<MooseFunctorName>("factor", 1, "A factor to apply on the functor");
  params.addParam<MaterialPropertyName>(
      "factor_matprop", 1, "A (regular) material property factor to apply on the functor");

  // We need some ghosting for the Finite Volume Fields (we use neighbor information to compute
  // gradient)
  params.addParam<unsigned short>("ghost_layers", 1, "The number of layers of elements to ghost.");
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        rm_params.set<unsigned short>("layers") = obj_params.get<unsigned short>("ghost_layers");
        rm_params.set<bool>("use_displaced_mesh") = obj_params.get<bool>("use_displaced_mesh");
      });
  return params;
}

template <bool is_ad>
FunctorElementalGradientAuxTempl<is_ad>::FunctorElementalGradientAuxTempl(
    const InputParameters & parameters)
  : VectorAuxKernel(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _factor(getFunctor<GenericReal<is_ad>>("factor")),
    _factor_matprop(getGenericMaterialProperty<Real, is_ad>("factor_matprop")),
    _use_qp_arg(dynamic_cast<MooseVariableFE<RealVectorValue> *>(&_var))
{
  if (!_use_qp_arg && !dynamic_cast<MooseVariableFV<Real> *>(&_var))
    paramError(
        "variable",
        "The variable must be a non-vector, non-array finite-volume/finite-element variable.");

  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

template <bool is_ad>
RealVectorValue
FunctorElementalGradientAuxTempl<is_ad>::computeValue()
{
  using MetaPhysicL::raw_value;
  const auto state = determineState();
  if (_use_qp_arg)
  {
    const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
    return raw_value(_factor(qp_arg, state)) * raw_value(_factor_matprop[_qp]) *
           raw_value(_functor.gradient(qp_arg, state));
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    mooseAssert(_qp == 0, "Only one Qp per element expected when using an elemental argument");
    return raw_value(_factor(elem_arg, state)) * raw_value(_factor_matprop[_qp]) *
           raw_value(_functor.gradient(elem_arg, state));
  }
}
