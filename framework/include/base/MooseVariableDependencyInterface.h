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
class MooseVariableFE;

class MooseVariableDependencyInterface
{
public:
  MooseVariableDependencyInterface() {}

  /**
   * Retrieve the set of MooseVariableFEs that _this_ object depends on.
   * @return The MooseVariableFEs that MUST be reinited before evaluating this object
   */
  const std::set<MooseVariableFE *> & getMooseVariableDependencies() const
  {
    return _moose_variable_dependencies;
  }

protected:
  /**
   * Call this function to add the passed in MooseVariableFE as a variable that _this_ object
   * depends on.
   */
  void addMooseVariableDependency(MooseVariableFE * var)
  {
    _moose_variable_dependencies.insert(var);
  }
  void addMooseVariableDependency(std::vector<MooseVariableFE *> vars)
  {
    _moose_variable_dependencies.insert(vars.begin(), vars.end());
  }

private:
  std::set<MooseVariableFE *> _moose_variable_dependencies;
};

#endif // MOOSEVARIABLEDEPENDENCYINTERFACE_H
