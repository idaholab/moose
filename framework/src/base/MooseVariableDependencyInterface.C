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

MooseVariableDependencyInterface::MooseVariableDependencyInterface()
{
  mooseDeprecated("The empty MaterialPropertyInterface constructor will be removed, please use the version that takes a MooseObject * as input.");
}


MooseVariableDependencyInterface::MooseVariableDependencyInterface(const MooseObject * /*moose_object*/)
{
}

const std::set<MooseVariable *> &
MooseVariableDependencyInterface::getMooseVariableDependencies()
{
  return _moose_variable_dependencies;
}


void
MooseVariableDependencyInterface::addMooseVariableDependency(MooseVariable * var)
{
  _moose_variable_dependencies.insert(var);
}


void MooseVariableDependencyInterface::addMooseVariableDependency(std::vector<MooseVariable *> vars)
{
  _moose_variable_dependencies.insert(vars.begin(), vars.end());
}
