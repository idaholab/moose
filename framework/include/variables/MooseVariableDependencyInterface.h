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

// Forward declarations
class MooseVariableFEBase;

class MooseVariableDependencyInterface
{
public:
  MooseVariableDependencyInterface() {}

  /**
   * Retrieve the set of MooseVariableFEBases that _this_ object depends on.
   * @return The MooseVariableFEBases that MUST be reinited before evaluating this object
   */
  const std::set<MooseVariableFEBase *> & getMooseVariableDependencies() const
  {
    return _moose_variable_dependencies;
  }

protected:
  /**
   * Call this function to add the passed in MooseVariableFEBase as a variable that _this_ object
   * depends on.
   */
  void addMooseVariableDependency(MooseVariableFEBase * var)
  {
    _moose_variable_dependencies.insert(var);
  }
  void addMooseVariableDependency(std::vector<MooseVariableFEBase *> vars)
  {
    _moose_variable_dependencies.insert(vars.begin(), vars.end());
  }

private:
  std::set<MooseVariableFEBase *> _moose_variable_dependencies;
};

