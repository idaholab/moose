//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMExecutedObject.h"

namespace Moose::MFEM
{
InputParameters
ExecutedObject::validParams()
{
  InputParameters params = Object::validParams();
  params += SetupInterface::validParams();
  params.registerSystemAttributeName("Moose::MFEM::ExecutedObject");
  params.addPrivateParam<std::vector<std::string>>("_mfem_dependency_param_names", {});
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_END;
  params.addClassDescription("Base class for executed MFEM objects.");
  return params;
}

ExecutedObject::ExecutedObject(const InputParameters & parameters)
  : Object(parameters), SetupInterface(this), DependencyResolverInterface()
{
}

std::optional<std::string>
ExecutedObject::suppliedVariableName() const
{
  return std::nullopt;
}

std::optional<std::string>
ExecutedObject::suppliedPostprocessorName() const
{
  return std::nullopt;
}

std::optional<std::string>
ExecutedObject::suppliedVectorPostprocessorName() const
{
  return std::nullopt;
}

const std::set<std::string> &
ExecutedObject::getRequestedItems()
{
  if (_requested_items)
    return *_requested_items;

  _requested_items.emplace();

  for (const auto & param : getParam<std::vector<std::string>>("_mfem_dependency_param_names"))
  {
    if (const auto * name = queryParam<VariableName>(param))
      _requested_items->insert(variableDependencyKey(*name));
    if (const auto * names = queryParam<std::vector<VariableName>>(param))
      for (const auto & name : *names)
        _requested_items->insert(variableDependencyKey(name));
    if (const auto * name = queryParam<PostprocessorName>(param))
      _requested_items->insert(postprocessorDependencyKey(*name));
    if (const auto * names = queryParam<std::vector<PostprocessorName>>(param))
      for (const auto & name : *names)
        _requested_items->insert(postprocessorDependencyKey(name));
    if (const auto * name = queryParam<VectorPostprocessorName>(param))
      _requested_items->insert(vectorPostprocessorDependencyKey(*name));
    if (const auto * names = queryParam<std::vector<VectorPostprocessorName>>(param))
      for (const auto & name : *names)
        _requested_items->insert(vectorPostprocessorDependencyKey(name));
  }

  return *_requested_items;
}

const std::set<std::string> &
ExecutedObject::getSuppliedItems()
{
  if (_supplied_items)
    return *_supplied_items;

  _supplied_items.emplace();

  if (const auto name = suppliedVariableName())
    _supplied_items->insert(variableDependencyKey(*name));
  if (const auto name = suppliedPostprocessorName())
    _supplied_items->insert(postprocessorDependencyKey(*name));
  if (const auto name = suppliedVectorPostprocessorName())
    _supplied_items->insert(vectorPostprocessorDependencyKey(*name));

  return *_supplied_items;
}

std::string
ExecutedObject::variableDependencyKey(const std::string & name)
{
  return "variable:" + name;
}

std::string
ExecutedObject::postprocessorDependencyKey(const std::string & name)
{
  return "postprocessor:" + name;
}

std::string
ExecutedObject::vectorPostprocessorDependencyKey(const std::string & name)
{
  return "vector_postprocessor:" + name;
}

void
ExecutedObject::appendDependencyParam(InputParameters & params, const std::string & param_name)
{
  auto & param_names = params.set<std::vector<std::string>>("_mfem_dependency_param_names");
  param_names.push_back(param_name);
}

} // namespace Moose::MFEM
#endif
