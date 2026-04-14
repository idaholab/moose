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

InputParameters
MFEMExecutedObject::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params += SetupInterface::validParams();
  params.registerSystemAttributeName("MFEMExecutedObject");
  params.addPrivateParam<std::vector<std::string>>("_mfem_dependency_param_names", {});
  params.addPrivateParam<std::vector<std::string>>("_mfem_dependency_param_kinds", {});
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_END;
  params.addClassDescription("Base class for executed MFEM objects.");
  return params;
}

MFEMExecutedObject::MFEMExecutedObject(const InputParameters & parameters)
  : MFEMObject(parameters), SetupInterface(this), DependencyResolverInterface()
{
  const auto & param_names = getParam<std::vector<std::string>>("_mfem_dependency_param_names");
  const auto & kinds = getParam<std::vector<std::string>>("_mfem_dependency_param_kinds");

  mooseAssert(param_names.size() == kinds.size(),
              "MFEM dependency parameter metadata size mismatch");

  _dependency_params.reserve(param_names.size());
  for (const auto i : index_range(param_names))
    _dependency_params.push_back({kinds[i], param_names[i]});
}

std::optional<std::string>
MFEMExecutedObject::suppliedVariableName() const
{
  return std::nullopt;
}

std::optional<std::string>
MFEMExecutedObject::suppliedPostprocessorName() const
{
  return std::nullopt;
}

std::optional<std::string>
MFEMExecutedObject::suppliedVectorPostprocessorName() const
{
  return std::nullopt;
}

const std::set<std::string> &
MFEMExecutedObject::getRequestedItems()
{
  if (_requested_items)
    return *_requested_items;

  _requested_items.emplace();

  for (const auto & dep : _dependency_params)
  {
    if (!isParamValid(dep.param_name))
      continue;

    if (dep.kind == typeid(VariableName).name())
      _requested_items->insert(variableDependencyKey(getParam<VariableName>(dep.param_name)));
    else if (dep.kind == typeid(std::vector<VariableName>).name())
      for (const auto & name : getParam<std::vector<VariableName>>(dep.param_name))
        _requested_items->insert(variableDependencyKey(name));
    else if (dep.kind == typeid(PostprocessorName).name())
      _requested_items->insert(
          postprocessorDependencyKey(getParam<PostprocessorName>(dep.param_name)));
    else if (dep.kind == typeid(std::vector<PostprocessorName>).name())
      for (const auto & name : getParam<std::vector<PostprocessorName>>(dep.param_name))
        _requested_items->insert(postprocessorDependencyKey(name));
    else if (dep.kind == typeid(VectorPostprocessorName).name())
      _requested_items->insert(
          vectorPostprocessorDependencyKey(getParam<VectorPostprocessorName>(dep.param_name)));
    else if (dep.kind == typeid(std::vector<VectorPostprocessorName>).name())
      for (const auto & name : getParam<std::vector<VectorPostprocessorName>>(dep.param_name))
        _requested_items->insert(vectorPostprocessorDependencyKey(name));
  }

  return *_requested_items;
}

const std::set<std::string> &
MFEMExecutedObject::getSuppliedItems()
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
MFEMExecutedObject::variableDependencyKey(const std::string & name)
{
  return "variable:" + name;
}

std::string
MFEMExecutedObject::postprocessorDependencyKey(const std::string & name)
{
  return "postprocessor:" + name;
}

std::string
MFEMExecutedObject::vectorPostprocessorDependencyKey(const std::string & name)
{
  return "vector_postprocessor:" + name;
}

void
MFEMExecutedObject::appendDependencyParam(InputParameters & params,
                                          const std::string & param_name,
                                          const std::string & kind)
{
  auto & param_names = params.set<std::vector<std::string>>("_mfem_dependency_param_names");
  auto & kinds = params.set<std::vector<std::string>>("_mfem_dependency_param_kinds");

#ifndef NDEBUG
  const auto it = std::find(param_names.begin(), param_names.end(), param_name);
  if (it != param_names.end())
  {
    const auto idx = std::distance(param_names.begin(), it);
    mooseAssert(kinds[idx] == kind,
                "MFEM dependency parameter metadata mismatch for parameter " + param_name);
  }
#endif

  param_names.push_back(param_name);
  kinds.push_back(kind);
}

#endif
