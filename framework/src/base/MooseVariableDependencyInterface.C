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

#include "MooseVariableDependencyInterface.h"
#include "MooseObject.h"
#include "MooseError.h"

MooseVariableDependencyInterface::MooseVariableDependencyInterface() :
    _variable_dependency_added(false)
{
  mooseDeprecated("The constructor for MooseVariableDependencyInterface has been updated to accept the MooseObject as an argument.");
}

MooseVariableDependencyInterface::MooseVariableDependencyInterface(const MooseObject * moose_object) :
    _mvdi_name(moose_object->name()),
    _variable_dependency_added(false)
{
}

const std::set<MooseVariable *> &
MooseVariableDependencyInterface::getMooseVariableDependencies()
{
  if (!_variable_dependency_added)
    mooseError("The object " << _mvdi_name << " is inheriting from MooseVariableDependencyInterface failed to add any variable dependencies, the 'addMooseVariableDependency' method must be called.");

  return _moose_variable_dependencies;
}


void
MooseVariableDependencyInterface::addMooseVariableDependency(MooseVariable * var)
{
  _variable_dependency_added = true;
  _moose_variable_dependencies.insert(var);
}


void MooseVariableDependencyInterface::addMooseVariableDependency(std::vector<MooseVariable *> vars)
{
  _variable_dependency_added = true;
  _moose_variable_dependencies.insert(vars.begin(), vars.end());
}
