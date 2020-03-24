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
VariableWarehouse::addBoundaryVar(BoundaryID bnd, MooseVariableFEBase * var)
{
  _boundary_vars[bnd].insert(var);
}

void
VariableWarehouse::addBoundaryVar(const std::set<BoundaryID> & boundary_ids,
                                  MooseVariableFEBase * var)
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
VariableWarehouse::getVariable(const std::string & var_name)
{
  return _var_name[var_name];
}

MooseVariableBase *
VariableWarehouse::getVariable(unsigned int var_number)
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

const std::set<MooseVariableFEBase *> &
VariableWarehouse::boundaryVars(BoundaryID bnd) const
{
  return _boundary_vars.find(bnd)->second;
}

template <typename T>
MooseVariableFE<T> *
VariableWarehouse::getFieldVariable(const std::string & var_name)
{
  // TODO: the requested variable might be an FV variable - how to we
  // reconcile this since this function returns an FE (not Field) pointer?
  // crap tons of objects depend on this.
  return _regular_vars_by_name.at(var_name);
}

template <typename T>
MooseVariableFE<T> *
VariableWarehouse::getFieldVariable(unsigned int var_number)
{
  // TODO: the requested variable might be an FV variable - how to we
  // reconcile this since this function returns an FE (not Field) pointer?
  // crap tons of objects depend on this.
  return _regular_vars_by_number.at(var_number);
}

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
VectorMooseVariable *
VariableWarehouse::getFieldVariable<RealVectorValue>(const std::string & var_name)
{
  // TODO: the requested variable might be an FV variable - how to we
  // reconcile this since this function returns an FE (not Field) pointer?
  // crap tons of objects depend on this.
  return _vector_vars_by_name.at(var_name);
}

template <>
VectorMooseVariable *
VariableWarehouse::getFieldVariable<RealVectorValue>(unsigned int var_number)
{
  // TODO: the requested variable might be an FV variable - how to we
  // reconcile this since this function returns an FE (not Field) pointer?
  // crap tons of objects depend on this.
  return _vector_vars_by_number.at(var_number);
}

template <>
ArrayMooseVariable *
VariableWarehouse::getFieldVariable<RealEigenVector>(const std::string & var_name)
{
  return _array_vars_by_name.at(var_name);
}

template <>
ArrayMooseVariable *
VariableWarehouse::getFieldVariable<RealEigenVector>(unsigned int var_number)
{
  return _array_vars_by_number.at(var_number);
}

template MooseVariableFE<Real> * VariableWarehouse::getFieldVariable<Real>(const std::string & var_name);
template MooseVariableFE<Real> * VariableWarehouse::getFieldVariable<Real>(unsigned int var_number);
template MooseVariableField<Real> * VariableWarehouse::getActualFieldVariable<Real>(const std::string & var_name);
template MooseVariableField<Real> * VariableWarehouse::getActualFieldVariable<Real>(unsigned int var_number);
