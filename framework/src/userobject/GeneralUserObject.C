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

#include "GeneralUserObject.h"

template<>
InputParameters validParams<GeneralUserObject>()
{
  InputParameters params = validParams<UserObject>();
  return params;
}

GeneralUserObject::GeneralUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    MaterialPropertyInterface(name, parameters),
    TransientInterface(parameters, name, "general_user_objects"),
    FunctionInterface(parameters),
    DependencyResolverInterface(),
    UserObjectInterface(parameters),
    PostprocessorInterface(parameters)
{
  _supplied_vars.insert(_name);
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

PostprocessorValue &
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
