//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorPropFromGradAndDot.h"

registerMooseObject("MooseTestApp", FunctorPropFromGradAndDot);

InputParameters
FunctorPropFromGradAndDot::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("functor",
                                            "The functor that will be queried for dot and grad in "
                                            "order to create the new functor material properties.");
  params.addRequiredParam<MaterialPropertyName>(
      "root_functor_prop_name", "The prefix for the functor properties declared here.");
  return params;
}

FunctorPropFromGradAndDot::FunctorPropFromGradAndDot(const InputParameters & parameters)
  : FunctorMaterial(parameters), _functor(getFunctor<ADReal>("functor"))
{
  addFunctorProperty<ADReal>(getParam<MaterialPropertyName>("root_functor_prop_name") + "_value",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _functor(r, t); });
  addFunctorProperty<ADRealVectorValue>(getParam<MaterialPropertyName>("root_functor_prop_name") +
                                            "_grad",
                                        [this](const auto & r, const auto & t) -> ADRealVectorValue
                                        { return _functor.gradient(r, t); });
  addFunctorProperty<ADReal>(getParam<MaterialPropertyName>("root_functor_prop_name") + "_dot",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _functor.dot(r, t); });
}
