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

const PostprocessorValue &
GeneralUserObject::getPostprocessorValue(const std::string & name, unsigned int index)
{
  // is this a vector of pp names, we use the number of default entries
  // to figure that out
  if (_pars.isSinglePostprocessor(name))
    _depend_vars.insert(_pars.get<PostprocessorName>(name));
  else
    _depend_vars.insert(_pars.get<std::vector<PostprocessorName>>(name)[index]);
  return UserObject::getPostprocessorValue(name, index);
}

const PostprocessorValue &
GeneralUserObject::getPostprocessorValueByName(const PostprocessorName & name)
{
  _depend_vars.insert(name);
  return UserObject::getPostprocessorValueByName(name);
}

const VectorPostprocessorValue &
GeneralUserObject::getVectorPostprocessorValue(const std::string & name,
                                               const std::string & vector_name)
{
  _depend_vars.insert(_pars.get<VectorPostprocessorName>(name));
  return UserObject::getVectorPostprocessorValue(name, vector_name);
}

const VectorPostprocessorValue &
GeneralUserObject::getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                                     const std::string & vector_name)
{
  _depend_vars.insert(name);
  return UserObject::getVectorPostprocessorValueByName(name, vector_name);
}

const VectorPostprocessorValue &
GeneralUserObject::getVectorPostprocessorValue(const std::string & name,
                                               const std::string & vector_name,
                                               bool use_broadcast)
{
  _depend_vars.insert(_pars.get<VectorPostprocessorName>(name));
  return UserObject::getVectorPostprocessorValue(name, vector_name, use_broadcast);
}

const VectorPostprocessorValue &
GeneralUserObject::getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                                     const std::string & vector_name,
                                                     bool use_broadcast)
{
  _depend_vars.insert(name);
  return UserObject::getVectorPostprocessorValueByName(name, vector_name, use_broadcast);
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
