//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <set>
#include <vector>

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
  MooseVariableDependencyInterface(const MooseObject * moose_object);

  /**
   * Retrieve the set of MooseVariableFieldBase that _this_ object depends on.
   * @return The MooseVariableFieldBase that MUST be reinited before evaluating this object
   */
  const std::set<MooseVariableFieldBase *> & getMooseVariableDependencies() const
  {
    return _moose_variable_dependencies;
  }

  /**
   * Check whether all of the variable dependencies are evaluable on the supplied degree of freedom
   * object
   */
  void checkEvaluable(const libMesh::DofObject & dof_object) const;

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

  /// The name of the moose object that this interface aggregates into
  const std::string & _mvdi_name;

  /// The type of the moose object that this interface aggregates into
  const std::string & _mvdi_type;
};
