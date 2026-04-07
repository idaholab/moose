//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMObject.h"
#include "SetupInterface.h"
#include "DependencyResolverInterface.h"

#include <algorithm>
#include <set>
#include <type_traits>

/**
 * Base class for MFEM objects that participate in execution ordering but are not UserObjects.
 */
class MFEMExecutedObject : public MFEMObject,
                           public SetupInterface,
                           public DependencyResolverInterface
{
public:
  enum class DependencyKind : unsigned char
  {
    Variable = 0,
    Postprocessor = 1,
    VectorPostprocessor = 2
  };

  struct DependencyParam
  {
    DependencyKind kind;
    std::string param_name;
    bool is_vector;
  };

  static InputParameters validParams();

  MFEMExecutedObject(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

  virtual std::set<std::string> producedVariableNames() const;
  virtual std::set<std::string> producedPostprocessorNames() const;
  virtual std::set<std::string> producedVectorPostprocessorNames() const;

  virtual const std::set<std::string> & getRequestedItems() override;
  virtual const std::set<std::string> & getSuppliedItems() override;

  template <typename T>
  static void addDependencyParam(InputParameters & params,
                                 const std::string & param_name,
                                 const std::string & doc_string);

  template <typename T>
  static void addRequiredDependencyParam(InputParameters & params,
                                         const std::string & param_name,
                                         const std::string & doc_string);

protected:
  static std::string variableDependencyKey(const std::string & name);
  static std::string postprocessorDependencyKey(const std::string & name);
  static std::string vectorPostprocessorDependencyKey(const std::string & name);

  template <typename T>
  static constexpr DependencyKind dependencyKind();

  template <typename T>
  static constexpr bool dependencyIsVector();

  static void appendDependencyParam(InputParameters & params,
                                    const std::string & param_name,
                                    const DependencyKind kind,
                                    const bool is_vector);

private:
  mutable std::set<std::string> _requested_items;
  mutable std::set<std::string> _supplied_items;
  std::vector<DependencyParam> _dependency_params;
};

template <typename T>
void
MFEMExecutedObject::addDependencyParam(InputParameters & params,
                                       const std::string & param_name,
                                       const std::string & doc_string)
{
  params.addParam<T>(param_name, doc_string);
  appendDependencyParam(params, param_name, dependencyKind<T>(), dependencyIsVector<T>());
}

template <typename T>
void
MFEMExecutedObject::addRequiredDependencyParam(InputParameters & params,
                                               const std::string & param_name,
                                               const std::string & doc_string)
{
  params.addRequiredParam<T>(param_name, doc_string);
  appendDependencyParam(params, param_name, dependencyKind<T>(), dependencyIsVector<T>());
}

template <typename T>
constexpr MFEMExecutedObject::DependencyKind
MFEMExecutedObject::dependencyKind()
{
  if constexpr (std::is_same_v<T, VariableName> || std::is_same_v<T, std::vector<VariableName>>)
    return DependencyKind::Variable;
  else if constexpr (std::is_same_v<T, PostprocessorName> ||
                     std::is_same_v<T, std::vector<PostprocessorName>>)
    return DependencyKind::Postprocessor;
  else if constexpr (std::is_same_v<T, VectorPostprocessorName> ||
                     std::is_same_v<T, std::vector<VectorPostprocessorName>>)
    return DependencyKind::VectorPostprocessor;
  else
    static_assert(!sizeof(T), "Unsupported MFEM executed-object dependency parameter type");
}

template <typename T>
constexpr bool
MFEMExecutedObject::dependencyIsVector()
{
  if constexpr (std::is_same_v<T, std::vector<VariableName>> ||
                std::is_same_v<T, std::vector<PostprocessorName>> ||
                std::is_same_v<T, std::vector<VectorPostprocessorName>>)
    return true;
  else if constexpr (std::is_same_v<T, VariableName> || std::is_same_v<T, PostprocessorName> ||
                     std::is_same_v<T, VectorPostprocessorName>)
    return false;
  else
    static_assert(!sizeof(T), "Unsupported MFEM executed-object dependency parameter type");
}

#endif
