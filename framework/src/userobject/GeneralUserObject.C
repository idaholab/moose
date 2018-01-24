//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralUserObject.h"

template <>
InputParameters
validParams<GeneralUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<MaterialPropertyInterface>();
  params.addParam<bool>(
      "force_preaux", false, "Forces the GeneralUserObject to be executed in PREAUX");
  params.addParamNamesToGroup("force_preaux", "Advanced");
  return params;
}

GeneralUserObject::GeneralUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    MaterialPropertyInterface(this),
    TransientInterface(this),
    DependencyResolverInterface(),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this)
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

const PostprocessorValue &
GeneralUserObject::getPostprocessorValue(const std::string & name)
{
  _depend_vars.insert(_pars.get<PostprocessorName>(name));
  return PostprocessorInterface::getPostprocessorValue(name);
}

const PostprocessorValue &
GeneralUserObject::getPostprocessorValueByName(const PostprocessorName & name)
{
  _depend_vars.insert(name);
  return PostprocessorInterface::getPostprocessorValueByName(name);
}

const VectorPostprocessorValue &
GeneralUserObject::getVectorPostprocessorValue(const std::string & name,
                                               const std::string & vector_name)
{
  _depend_vars.insert(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getVectorPostprocessorValue(name, vector_name);
}

const VectorPostprocessorValue &
GeneralUserObject::getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                                     const std::string & vector_name)
{
  _depend_vars.insert(name);
  return VectorPostprocessorInterface::getVectorPostprocessorValueByName(name, vector_name);
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
