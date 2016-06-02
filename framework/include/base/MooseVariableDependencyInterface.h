/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSEVARIABLEDEPENDENCYINTERFACE_H
#define MOOSEVARIABLEDEPENDENCYINTERFACE_H

#include <set>
#include <vector>

// Forward declarations
class MooseVariable;

class MooseVariableDependencyInterface
{
public:
  MooseVariableDependencyInterface();

  /**
   * Retrieve the set of MooseVariables that _this_ object depends on.
   * @return The MooseVariables that MUST be reinited before evaluating this object
   */
  const std::set<MooseVariable *> & getMooseVariableDependencies();

protected:

  ///@{
  /**
   * Call this function to add the passed in MooseVariable(s) as a variable that _this_ object depends on.
   */
  void addMooseVariableDependency(MooseVariable * var);
  void addMooseVariableDependency(std::vector<MooseVariable *> vars);
  ///@}

private:

  /// The set of variables that this object depends on.
  std::set<MooseVariable *> _moose_variable_dependencies;

  /// A flag to produce a warning if the add method is not called.
  bool _variable_dependency_added;

};

#endif // MOOSEVARIABLEDEPENDENCYINTERFACE_H
