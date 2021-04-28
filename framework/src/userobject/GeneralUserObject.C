//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralUserObject.h"

defineLegacyParams(GeneralUserObject);

InputParameters
GeneralUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += MaterialPropertyInterface::validParams();
  params.addParam<bool>(
      "force_preaux", false, "Forces the GeneralUserObject to be executed in PREAUX");
  params.addParam<bool>(
      "force_preic",
      false,
      "Forces the GeneralUserObject to be executed in PREIC during initial setup");
  params.addParamNamesToGroup("force_preaux force_preic", "Advanced");
  return params;
}

GeneralUserObject::GeneralUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS),
    TransientInterface(this),
    DependencyResolverInterface()
{
  _supplied_vars.insert(name());
}

const std::set<std::string> &
GeneralUserObject::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
GeneralUserObject::getSuppliedItems()
{
  return _supplied_vars;
}

void
GeneralUserObject::addPostprocessorDependencyHelper(const PostprocessorName & name) const
{
  UserObject::addPostprocessorDependencyHelper(name);
  _depend_vars.insert(name);
}

void
GeneralUserObject::addVectorPostprocessorDependencyHelper(
    const VectorPostprocessorName & name) const
{
  UserObject::addVectorPostprocessorDependencyHelper(name);
  _depend_vars.insert(name);
}

void
GeneralUserObject::threadJoin(const UserObject &)
{
  mooseError("GeneralUserObjects do not execute using threads, this function does nothing and "
             "should not be used.");
}

void
GeneralUserObject::subdomainSetup()
{
  mooseError("GeneralUserObjects do not execute subdomainSetup method, this function does nothing "
             "and should not be used.");
}

void
GeneralUserObject::addReporterDependencyHelper(const ReporterName & reporter_name)
{
  _depend_vars.insert(reporter_name.getObjectName());
}
