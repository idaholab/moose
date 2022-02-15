//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorInterface.h"
#include "MooseFunctor.h"

InputParameters
FunctorInterface::validParams()
{
  return emptyInputParameters();
}

FunctorInterface::FunctorInterface(const MooseObject * const moose_object)
  : _fi_params(moose_object->parameters()),
    _fi_name(_fi_params.get<std::string>("_object_name")),
    _fi_subproblem(*_fi_params.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fi_tid(_fi_params.get<THREAD_ID>("_tid"))
{
}

std::string
FunctorInterface::deduceFunctorName(const std::string & name, const InputParameters & params)
{
  if (params.isParamValid(name))
  {
    if (params.have_parameter<MooseFunctorName>(name))
      return params.get<MooseFunctorName>(name);
    // variables, functor material properties, and functions are also functors
    else if (params.have_parameter<MaterialPropertyName>(name))
      return params.get<MaterialPropertyName>(name);
    else if (params.have_parameter<VariableName>(name))
      return params.get<VariableName>(name);
    else if (params.have_parameter<std::vector<VariableName>>(name))
    {
      const auto & var_names = params.get<std::vector<VariableName>>(name);
      if (var_names.size() != 1)
        mooseError("We only support a single variable name for retrieving a functor");
      return var_names[0];
    }
    else if (params.have_parameter<NonlinearVariableName>(name))
      return params.get<NonlinearVariableName>(name);
    else if (params.have_parameter<FunctionName>(name))
      return params.get<FunctionName>(name);
    else
      mooseError("Invalid parameter type for retrieving a functor");
  }
  else
    return name;
}

std::string
FunctorInterface::deduceFunctorName(const std::string & name) const
{
  return deduceFunctorName(name, _fi_params);
}

template <>
const Moose::Functor<Real> *
FunctorInterface::defaultFunctor(const std::string & name)
{
  std::istringstream ss(name);
  Real real_value;

  // check if the string parsed cleanly into a Real number
  if (ss >> real_value && ss.eof())
  {
    _default_real_functors.emplace_back(std::make_unique<Moose::Functor<Real>>(
        std::make_unique<Moose::ConstantFunctor<Real>>(real_value)));
    auto & default_property = _default_real_functors.back();
    return default_property.get();
  }

  return nullptr;
}

template <>
const Moose::Functor<ADReal> *
FunctorInterface::defaultFunctor(const std::string & name)
{
  std::istringstream ss(name);
  Real real_value;

  // check if the string parsed cleanly into a Real number
  if (ss >> real_value && ss.eof())
  {
    _default_ad_real_functors.emplace_back(std::make_unique<Moose::Functor<ADReal>>(
        std::make_unique<Moose::ConstantFunctor<ADReal>>(real_value)));
    auto & default_property = _default_ad_real_functors.back();
    return default_property.get();
  }

  return nullptr;
}

bool
FunctorInterface::isFunctor(const std::string & name) const
{
  // Check if the supplied parameter is a valid input parameter key
  std::string functor_name = deduceFunctorName(name);

  return _fi_subproblem.hasFunctor(functor_name, _fi_tid);
}

Moose::ElemArg
FunctorInterface::makeElemArg(const Elem * const elem, const bool correct_skewness) const
{
  return {elem, correct_skewness, /*apply_gradient_to_skewness=*/correct_skewness};
}
