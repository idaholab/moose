//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "HashMap.h"

#include <vector>
#include <map>
#include <set>

namespace libMesh
{
template <typename>
class VectorValue;
typedef VectorValue<Real> RealVectorValue;
}

class MooseVariableBase;
class MooseVariableFieldBase;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<RealVectorValue> VectorMooseVariable;
typedef MooseVariableFE<RealEigenVector> ArrayMooseVariable;

template <typename>
class MooseVariableFV;
typedef MooseVariableFV<Real> MooseVariableFVReal;

class MooseVariableScalar;

/**
 * Holds variables and provides some services
 */
class VariableWarehouse
{
public:
  VariableWarehouse();

  /**
   * Add a variable
   * @param var_name The name of the variable
   * @param var Variable
   */
  void add(const std::string & var_name, std::shared_ptr<MooseVariableBase> var);

  /**
   * Add a boundary variable
   * @param bnd The boundary id where this variable is defined
   * @param var The variable
   */
  void addBoundaryVar(BoundaryID bnd, MooseVariableFieldBase * var);

  /**
   * Add a variable to a set of boundaries
   * @param boundary_ids The boundary ids where this variable is defined
   * @param var The variable
   */
  void addBoundaryVar(const std::set<BoundaryID> & boundary_ids, MooseVariableFieldBase * var);

  /**
   * Add a map of variables to a set of boundaries
   * @param boundary_ids The boundary ids where this variable is defined
   * @param vars A map of variables
   */
  void
  addBoundaryVars(const std::set<BoundaryID> & boundary_ids,
                  const std::unordered_map<std::string, std::vector<MooseVariableFieldBase *>> & vars);

  /**
   * Get a variable from the warehouse
   * @param var_name The name of the variable to retrieve
   * @return The retrieved variable
   */
  MooseVariableBase * getVariable(const std::string & var_name);

  /**
   * Get a variable from the warehouse
   * @param var_number The number of the variable to retrieve
   * @return The retrieved variable
   */
  MooseVariableBase * getVariable(unsigned int var_number);

  /**
   * Get a finite element variable from the warehouse
   * of either Real or RealVectorValue type
   * @param var_name The name of the variable to retrieve
   * @return The retrieved variable
   */
  template <typename T>
  MooseVariableFE<T> * getFieldVariable(const std::string & var_name);

  /**
   * Get a finite element variable from the warehouse
   * of either Real or RealVectorValue type
   * @param var_number The number of the variable to retrieve
   * @return The retrieved variable
   */
  template <typename T>
  MooseVariableFE<T> * getFieldVariable(unsigned int var_number);

  /**
   * Get the list of all variable names
   * @return The list of variable names
   */
  const std::vector<VariableName> & names() const;

  /**
   * Get the list of variables
   * @return The list of variables
   */
  const std::vector<MooseVariableFieldBase *> & fieldVariables() const;

  /**
   * Get the list of variables that needs to be reinitialized on a given boundary
   * @param bnd The boundary ID
   * @return The list of variables
   */
  const std::set<MooseVariableFieldBase *> & boundaryVars(BoundaryID bnd) const;

  /**
   * Get the list of scalar variables
   * @return The list of scalar variables
   */
  const std::vector<MooseVariableScalar *> & scalars() const;

protected:
  /// list of variable names
  std::vector<VariableName> _names;

  /// list of finite element variables
  std::vector<MooseVariableFieldBase *> _vars;

  /// map of non-vector finite element variables with unsigned keys
  HashMap<unsigned, MooseVariable *> _regular_vars_by_number;

  /// map of non-vector finite element variables with name keys
  HashMap<std::string, MooseVariableFVReal *> _fv_vars_by_name;

  /// map of non-vector finite element variables with name keys
  HashMap<std::string, MooseVariable *> _regular_vars_by_name;

  /// map of non-vector finite element variables with unsigned keys
  HashMap<unsigned, MooseVariableFVReal *> _fv_vars_by_number;

  /// map of vector finite element variables with name keys
  HashMap<std::string, VectorMooseVariable *> _vector_vars_by_name;

  /// map of vector finite element variables with unsigned keys
  HashMap<unsigned, VectorMooseVariable *> _vector_vars_by_number;

  /// map of vector finite element variables with name keys
  HashMap<std::string, ArrayMooseVariable *> _array_vars_by_name;

  /// map of vector finite element variables with unsigned keys
  HashMap<unsigned, ArrayMooseVariable *> _array_vars_by_number;

  /// Name to variable mapping
  std::map<std::string, MooseVariableBase *> _var_name;

  /// Map to variables that need to be evaluated on a boundary
  std::map<BoundaryID, std::set<MooseVariableFieldBase *>> _boundary_vars;

  /// list of all scalar, non-finite element variables
  std::vector<MooseVariableScalar *> _scalar_vars;

  /// All instances of objects (raw pointers)
  std::map<unsigned int, std::shared_ptr<MooseVariableBase>> _all_objects;
};

template <>
MooseVariableFE<RealVectorValue> *
VariableWarehouse::getFieldVariable<RealVectorValue>(const std::string & var_name);

template <>
MooseVariableFE<RealVectorValue> *
VariableWarehouse::getFieldVariable<RealVectorValue>(unsigned int var_number);

template <>
MooseVariableFE<RealEigenVector> *
VariableWarehouse::getFieldVariable<RealEigenVector>(const std::string & var_name);

template <>
MooseVariableFE<RealEigenVector> *
VariableWarehouse::getFieldVariable<RealEigenVector>(unsigned int var_number);
