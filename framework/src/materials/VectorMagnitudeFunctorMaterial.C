//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorMagnitudeFunctorMaterial.h"
#include "Function.h"

registerMooseObject("MooseApp", VectorMagnitudeFunctorMaterial);
registerMooseObject("MooseApp", ADVectorMagnitudeFunctorMaterial);

template <bool is_ad>
InputParameters
VectorMagnitudeFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};
  params.addClassDescription(
      "This class takes up to three scalar-valued functors corresponding to vector "
      "components *or* a single vector functor and computes the Euclidean norm.");
  params.addParam<MooseFunctorName>("x_functor", "The functor corresponding to the x component.");
  params.addParam<MooseFunctorName>(
      "y_functor", 0, "The functor corresponding to the y component.");
  params.addParam<MooseFunctorName>(
      "z_functor", 0, "The functor corresponding to the z component.");
  params.addRequiredParam<MooseFunctorName>(
      "vector_magnitude_name", "The name of the vector magnitude functor that we are creating.");
  params.addParam<MooseFunctorName>(
      "vector_functor", "The name of a vector functor that we will take the magnitude of.");
  return params;
}

template <bool is_ad>
VectorMagnitudeFunctorMaterialTempl<is_ad>::VectorMagnitudeFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _x(isParamValid("x_functor") ? &getFunctor<GenericReal<is_ad>>("x_functor") : nullptr),
    _y(getFunctor<GenericReal<is_ad>>("y_functor")),
    _z(getFunctor<GenericReal<is_ad>>("z_functor")),
    _vector_functor(isParamValid("vector_functor")
                        ? &getFunctor<VectorValue<GenericReal<is_ad>>>("vector_functor")
                        : nullptr)
{
  auto check_error =
      [this, &parameters](const std::string & scalar_functor, const bool must_equal_one)
  {
    auto error_message = [&scalar_functor, this]()
    {
      mooseError("Either a '",
                 scalar_functor,
                 "' or 'vector_functor' parameter "
                 "must be provided to '",
                 name(),
                 "'");
    };
    const unsigned short sum =
        parameters.isParamSetByUser(scalar_functor) + parameters.isParamSetByUser("vector_functor");
    if (must_equal_one)
    {
      if (sum != 1)
        error_message();
    }
    else
    {
      if (sum > 1)
        error_message();
    }
  };

  check_error("x_functor", true);
  check_error("y_functor", false);
  check_error("z_functor", false);

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  if (isParamValid("x_functor"))
    addFunctorProperty<GenericReal<is_ad>>(
        "vector_magnitude_name",
        [this](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          const auto x = (*_x)(r, t);
          const auto y = _y(r, t);
          const auto z = _z(r, t);
          return std::sqrt((x * x) + (y * y) + (z * z));
        },
        clearance_schedule);
  else
    addFunctorProperty<GenericReal<is_ad>>(
        "vector_magnitude_name",
        [this](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          const auto vec = (*_vector_functor)(r, t);
          return vec.norm();
        },
        clearance_schedule);
}

template class VectorMagnitudeFunctorMaterialTempl<false>;
template class VectorMagnitudeFunctorMaterialTempl<true>;
