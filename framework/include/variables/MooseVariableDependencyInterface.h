//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/id_types.h"

#include <set>
#include <vector>
#include <algorithm>

class MooseObject;
class MooseVariableFieldBase;
namespace libMesh
{
class DofObject;
}

class MooseVariableDependencyInterface
{
public:
  // Must be a pointer in order to disambiguate with default copy constructor
  MooseVariableDependencyInterface(const MooseObject *);

  /**
   * Retrieve the set of MooseVariableFieldBase that _this_ object depends on.
   * @return The MooseVariableFieldBase that MUST be reinited before evaluating this object
   */
  const std::set<MooseVariableFieldBase *> & getMooseVariableDependencies() const
  {
    return _moose_variable_dependencies;
  }

  /**
   * Check whether all of the variable dependencies have degree of freedom indices on the supplied
   * degree of freedom object
   * @param dof_object The degree of freedom object (an element or node) that we want to check for
   * existence of variable degrees of freedom on
   * @param vars_to_omit Variables that we can omit from checking
   * @return Any variables that do not have degrees of freedom on the supplied degree of freedom
   * object
   */
  template <typename DofObjectType>
  std::set<MooseVariableFieldBase *>
  checkAllVariables(const DofObjectType & dof_object,
                    const std::set<MooseVariableFieldBase *> & vars_to_omit = {});

  /**
   * Check whether all of the supplied variables have degree of freedom indices on the supplied
   * degree of freedom object
   * @param dof_object The degree of freedom object (an element or node) that we want to check for
   * existence of variable degrees of freedom on
   * @param vars_to_check the variables to check
   * @return Any variables that do not have degrees of freedom on the supplied degree of freedom
   * object
   */
  template <typename DofObjectType>
  std::set<MooseVariableFieldBase *>
  checkVariables(const DofObjectType & dof_object,
                 const std::set<MooseVariableFieldBase *> & vars_to_check);

protected:
  /**
   * Call this function to add the passed in MooseVariableFieldBase as a variable that _this_ object
   * depends on.
   */
  void addMooseVariableDependency(MooseVariableFieldBase * var)
  {
    _moose_variable_dependencies.insert(var);
  }
  void addMooseVariableDependency(const std::vector<MooseVariableFieldBase *> & vars)
  {
    _moose_variable_dependencies.insert(vars.begin(), vars.end());
  }

private:
  std::set<MooseVariableFieldBase *> _moose_variable_dependencies;

  /// A container for holding dof indices in order to avoid constant memory reallocation
  std::vector<libMesh::dof_id_type> _dof_indices;
};

template <typename DofObjectType>
std::set<MooseVariableFieldBase *>
MooseVariableDependencyInterface::checkAllVariables(
    const DofObjectType & dof_object, const std::set<MooseVariableFieldBase *> & vars_to_omit)
{
  if (vars_to_omit.empty())
    return checkVariables(dof_object, _moose_variable_dependencies);

  std::set<MooseVariableFieldBase *> vars_to_check;
  std::set_difference(_moose_variable_dependencies.begin(),
                      _moose_variable_dependencies.end(),
                      vars_to_omit.begin(),
                      vars_to_omit.end(),
                      std::inserter(vars_to_check, vars_to_check.begin()));
  return checkVariables(dof_object, vars_to_check);
}
