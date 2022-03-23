//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InstantiateFancyFunctorProps.h"

registerMooseObject("MooseTestApp", InstantiateFancyFunctorProps);

InputParameters
InstantiateFancyFunctorProps::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  return params;
}

InstantiateFancyFunctorProps::InstantiateFancyFunctorProps(const InputParameters & parameters)
  : FunctorMaterial(parameters)
{
  addFunctorProperty<std::array<ADReal, 1>>(
      "std_array_prop",
      [this](const auto &, const auto &) -> std::array<ADReal, 1> { return {0}; });
  addFunctorProperty<std::vector<ADReal>>(
      "std_vector_prop", [this](const auto &, const auto &) -> std::vector<ADReal> { return {0}; });
}
