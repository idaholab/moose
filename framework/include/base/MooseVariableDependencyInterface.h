//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEVARIABLEDEPENDENCYINTERFACE_H
#define MOOSEVARIABLEDEPENDENCYINTERFACE_H

#include <set>

// Forward declarations
class MooseVariable;

class MooseVariableDependencyInterface
{
public:
  MooseVariableDependencyInterface() {}

  /**
   * Retrieve the set of MooseVariables that _this_ object depends on.
   * @return The MooseVariables that MUST be reinited before evaluating this object
   */
  const std::set<MooseVariable *> & getMooseVariableDependencies() const
  {
    return _moose_variable_dependencies;
  }

protected:
  /**
   * Call this function to add the passed in MooseVariable as a variable that _this_ object depends
   * on.
   */
  void addMooseVariableDependency(MooseVariable * var) { _moose_variable_dependencies.insert(var); }
  void addMooseVariableDependency(std::vector<MooseVariable *> vars)
  {
    _moose_variable_dependencies.insert(vars.begin(), vars.end());
  }

private:
  std::set<MooseVariable *> _moose_variable_dependencies;
};

#endif // MOOSEVARIABLEDEPENDENCYINTERFACE_H
