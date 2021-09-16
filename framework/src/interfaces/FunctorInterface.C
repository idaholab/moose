//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorInterface.h"

InputParameters
FunctorInterface::validParams()
{
  return emptyInputParameters();
}

FunctorInterface::FunctorInterface(const MooseObject * const moose_object)
  : _fi_params(moose_object->parameters()),
    _fi_name(_mi_params.get<std::string>("_object_name")),
    _fi_subproblem(*_fi_params.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fi_tid(_fi_params.get<THREAD_ID>("_tid"))
{
}

std::string
FunctorInterface::deduceFunctorName(const std::string & name) const
{
  if (_fi_params.have_parameter<MooseFunctorName>(name))
    return _fi_params.get<MooseFunctorName>(name);
  else
    return name;
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
    _default_real_functors.emplace_back(
        libmesh_make_unique<Moose::ConstantFunctor<Real>>(real_value));
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
    _default_ad_real_functors.emplace_back(
        libmesh_make_unique<Moose::ConstantFunctor<ADReal>>(real_value));
    auto & default_property = _default_ad_real_functors.back();
    return default_property.get();
  }

  return nullptr;
}
