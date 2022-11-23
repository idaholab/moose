//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableWarehouse.h"
#include "MooseVariableFE.h"
#include "MooseVariableFV.h"
#include "MooseVariableScalar.h"
#include "MooseTypes.h"

VariableWarehouse::VariableWarehouse() {}

void
VariableWarehouse::add(const std::string & var_name, std::shared_ptr<MooseVariableBase> var)
{
  _names.push_back(var_name);
  auto * raw_var = var.get();
  _all_objects[var->number()] = var;
  _var_name[var_name] = raw_var;

  if (auto * tmp_var = dynamic_cast<MooseVariableFieldBase *>(raw_var))
  {
    _vars.push_back(tmp_var);
    if (auto * tmp_var = dynamic_cast<MooseVariable *>(raw_var))
    {
      _regular_vars_by_number[tmp_var->number()] = tmp_var;
      _regular_vars_by_name[var_name] = tmp_var;
    }
    else if (auto * tmp_var = dynamic_cast<MooseVariableFVReal *>(raw_var))
    {
      _fv_vars_by_number[tmp_var->number()] = tmp_var;
      _fv_vars_by_name[var_name] = tmp_var;
    }
    else if (auto * tmp_var = dynamic_cast<VectorMooseVariable *>(raw_var))
    {
      _vector_vars_by_number[tmp_var->number()] = tmp_var;
      _vector_vars_by_name[var_name] = tmp_var;
    }
    else if (auto * tmp_var = dynamic_cast<ArrayMooseVariable *>(raw_var))
    {
      _array_vars_by_number[tmp_var->number()] = tmp_var;
      _array_vars_by_name[var_name] = tmp_var;
    }
    else
      mooseError("Unknown variable class passed into VariableWarehouse. Attempt to hack us?");
  }
  else if (auto * tmp_var = dynamic_cast<MooseVariableScalar *>(raw_var))
    _scalar_vars.push_back(tmp_var);
  else
    mooseError("Unknown variable class passed into VariableWarehouse. Attempt to hack us?");
}

void
VariableWarehouse::addBoundaryVar(BoundaryID bnd, const MooseVariableFEBase * var)
{
  _boundary_vars[bnd].insert(var);
}

void
VariableWarehouse::addBoundaryVar(const std::set<BoundaryID> & boundary_ids,
                                  const MooseVariableFEBase * var)
{
  for (const auto & bid : boundary_ids)
    addBoundaryVar(bid, var);
}

void
VariableWarehouse::addBoundaryVars(
    const std::set<BoundaryID> & boundary_ids,
    const std::unordered_map<std::string, std::vector<MooseVariableFEBase *>> & vars)
{
  for (const auto & bid : boundary_ids)
    for (const auto & it : vars)
      for (const auto & var : it.second)
        addBoundaryVar(bid, var);
}

MooseVariableBase *
VariableWarehouse::getVariable(const std::string & var_name) const
{
  auto it = _var_name.find(var_name);
  if (it != _var_name.end())
    return it->second;
  else
    return nullptr;
}

MooseVariableBase *
VariableWarehouse::getVariable(unsigned int var_number) const
{
  auto it = _all_objects.find(var_number);
  if (it != _all_objects.end())
    return it->second.get();
  else
    return nullptr;
}

const std::vector<VariableName> &
VariableWarehouse::names() const
{
  return _names;
}

const std::vector<MooseVariableFEBase *> &
VariableWarehouse::fieldVariables() const
{
  return _vars;
}

const std::vector<MooseVariableScalar *> &
VariableWarehouse::scalars() const
{
  return _scalar_vars;
}

const std::set<const MooseVariableFEBase *> &
VariableWarehouse::boundaryVars(BoundaryID bnd) const
{
  return _boundary_vars.find(bnd)->second;
}

namespace
{
template <typename T, typename Map, typename Key>
MooseVariableFE<T> *
getFieldVariableHelper(const Map & map, const Key & key, const std::string & container_name)
{
  // TODO: the requested variable might be an FV variable - how to we
  // reconcile this since this function returns an FE (not Field) pointer?
  // crap tons of objects depend on this.

  auto it = map.find(key);
  if (it == map.end())
    mooseError("Key '", key, "' not found in VariableWarehouse container '", container_name, "'");

  return it->second;
}
}

template <typename T>
MooseVariableFE<T> *
VariableWarehouse::getFieldVariable(const std::string & var_name)
{
  return getFieldVariableHelper<T>(_regular_vars_by_name, var_name, "_regular_vars_by_name");
}

template <typename T>
MooseVariableFE<T> *
VariableWarehouse::getFieldVariable(unsigned int var_number)
{
  return getFieldVariableHelper<T>(_regular_vars_by_number, var_number, "_regular_vars_by_number");
}

template <>
VectorMooseVariable *
VariableWarehouse::getFieldVariable<RealVectorValue>(const std::string & var_name)
{
  return getFieldVariableHelper<RealVectorValue>(
      _vector_vars_by_name, var_name, "_vector_vars_by_name");
}

template <>
VectorMooseVariable *
VariableWarehouse::getFieldVariable<RealVectorValue>(unsigned int var_number)
{
  return getFieldVariableHelper<RealVectorValue>(
      _vector_vars_by_number, var_number, "_vector_vars_by_number");
}

template <>
ArrayMooseVariable *
VariableWarehouse::getFieldVariable<RealEigenVector>(const std::string & var_name)
{
  return getFieldVariableHelper<RealEigenVector>(
      _array_vars_by_name, var_name, "_array_vars_by_name");
}

template <>
ArrayMooseVariable *
VariableWarehouse::getFieldVariable<RealEigenVector>(unsigned int var_number)
{
  return getFieldVariableHelper<RealEigenVector>(
      _array_vars_by_number, var_number, "_array_vars_by_number");
}

template MooseVariableFE<Real> *
VariableWarehouse::getFieldVariable<Real>(const std::string & var_name);
template MooseVariableFE<Real> * VariableWarehouse::getFieldVariable<Real>(unsigned int var_number);

template <typename T>
MooseVariableField<T> *
VariableWarehouse::getActualFieldVariable(const std::string & var_name)
{
  auto it = _regular_vars_by_name.find(var_name);
  if (it != _regular_vars_by_name.end())
    return it->second;
  return _fv_vars_by_name.at(var_name);
}

template <typename T>
MooseVariableField<T> *
VariableWarehouse::getActualFieldVariable(unsigned int var_number)
{
  auto it = _regular_vars_by_number.find(var_number);
  if (it != _regular_vars_by_number.end())
    return it->second;
  return _fv_vars_by_number.at(var_number);
}

template <>
MooseVariableField<RealVectorValue> *
VariableWarehouse::getActualFieldVariable<RealVectorValue>(const std::string & var_name)
{
  // TODO: when necessary, add the if check to see if we have an FV vector var
  // before just returning nothing as found in FE vars list.
  return getFieldVariable<RealVectorValue>(var_name);
}

template <>
MooseVariableField<RealVectorValue> *
VariableWarehouse::getActualFieldVariable<RealVectorValue>(unsigned int var_number)
{
  // TODO: when necessary, add the if check to see if we have an FV vector var
  // before just returning nothing as found in FE vars list.
  return getFieldVariable<RealVectorValue>(var_number);
}

template <>
MooseVariableField<RealEigenVector> *
VariableWarehouse::getActualFieldVariable<RealEigenVector>(const std::string & var_name)
{
  return getFieldVariable<RealEigenVector>(var_name);
}

template <>
MooseVariableField<RealEigenVector> *
VariableWarehouse::getActualFieldVariable<RealEigenVector>(unsigned int var_number)
{
  return getFieldVariable<RealEigenVector>(var_number);
}

void
VariableWarehouse::initialSetup()
{
  for (auto & pair : _all_objects)
    pair.second->initialSetup();
}

void
VariableWarehouse::timestepSetup()
{
  for (auto & pair : _all_objects)
    pair.second->timestepSetup();
}

void
VariableWarehouse::customSetup(const ExecFlagType & exec_type)
{
  for (auto & pair : _all_objects)
    pair.second->customSetup(exec_type);
}

void
VariableWarehouse::subdomainSetup()
{
  for (auto & pair : _all_objects)
    pair.second->subdomainSetup();
}

void
VariableWarehouse::jacobianSetup()
{
  for (auto & pair : _all_objects)
    pair.second->jacobianSetup();
}

void
VariableWarehouse::residualSetup()
{
  for (auto & pair : _all_objects)
    pair.second->residualSetup();
}

void
VariableWarehouse::clearAllDofIndices()
{
  for (auto * var : _vars)
    var->clearAllDofIndices();
}

void
VariableWarehouse::setActiveVariableCoupleableVectorTags(const std::set<TagID> & vtags)
{
  for (auto * var : _vars)
    var->setActiveTags(vtags);
}

void
VariableWarehouse::setActiveScalarVariableCoupleableVectorTags(const std::set<TagID> & vtags)
{
  for (auto * var : _scalar_vars)
    var->setActiveTags(vtags);
}

template MooseVariableField<Real> *
VariableWarehouse::getActualFieldVariable<Real>(const std::string & var_name);
template MooseVariableField<Real> *
VariableWarehouse::getActualFieldVariable<Real>(unsigned int var_number);
