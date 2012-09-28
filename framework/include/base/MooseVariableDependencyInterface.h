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

class MooseVariableDependencyInterface
{
public:
  MooseVariableDependencyInterface() {}

  /**
   * Retrieve the set of MooseVariables that _this_ object depends on.
   * @return The MooseVariables that MUST be reinited before evaluating this object
   */
  const std::set<MooseVariable *> & getMooseVariableDependencies() { return _moose_variable_dependencies; }

protected:

  /**
   * Call this function ot add the passed in MooseVariable as a variable that _this_ object depends on.
   */
  void addMooseVariableDependency(MooseVariable * var) { _moose_variable_dependencies.insert(var); }

private:
  std::set<MooseVariable *> _moose_variable_dependencies;
};

#endif // MOOSEVARIABLEDEPENDENCYINTERFACE_H
