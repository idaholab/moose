//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserObjectInterface.h"

// MOOSE includes
#include "FEProblemBase.h"
#include "DiscreteElementUserObject.h"
#include "ThreadedGeneralUserObject.h"

InputParameters
UserObjectInterface::validParams()
{
  return emptyInputParameters();
}

UserObjectInterface::UserObjectInterface(const MooseObject * moose_object)
  : _uoi_moose_object(*moose_object),
    _uoi_feproblem(*_uoi_moose_object.parameters().getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base")),
    _uoi_tid(_uoi_moose_object.parameters().have_parameter<THREAD_ID>("_tid")
                 ? _uoi_moose_object.parameters().get<THREAD_ID>("_tid")
                 : 0)
{
}

UserObjectName
UserObjectInterface::getUserObjectName(const std::string & param_name) const
{
  const auto & params = _uoi_moose_object.parameters();

  if (!params.isParamValid(param_name))
    _uoi_moose_object.mooseError("Failed to get a parameter with the name \"",
                                 param_name,
                                 "\" when getting a UserObjectName.",
                                 "\n\nKnown parameters:\n",
                                 _uoi_moose_object.parameters());

  // Other interfaces will use this interface (PostprocessorInterface, VectorPostprocessorInterface)
  // to grab UOs with a specialized name, so we need to check them all
  UserObjectName name;
  if (params.isType<UserObjectName>(param_name))
    name = params.get<UserObjectName>(param_name);
  else if (params.isType<PostprocessorName>(param_name))
    name = params.get<PostprocessorName>(param_name);
  else if (params.isType<VectorPostprocessorName>(param_name))
    name = params.get<VectorPostprocessorName>(param_name);
  else if (params.isType<std::string>(param_name))
    name = params.get<std::string>(param_name);
  else
    _uoi_moose_object.paramError(
        param_name,
        "Parameter of type \"",
        params.type(param_name),
        "\" is not an expected type for getting the name of a UserObject.");

  return name;
}

bool
UserObjectInterface::hasUserObject(const std::string & param_name) const
{
  return hasUserObjectByName(getUserObjectName(param_name));
}

bool
UserObjectInterface::hasUserObjectByName(const UserObjectName & object_name) const
{
  return _uoi_feproblem.hasUserObject(object_name);
}

const UserObject &
UserObjectInterface::getUserObjectBase(const std::string & param_name,
                                       const bool is_dependency) const
{
  const auto object_name = getUserObjectName(param_name);
  if (!hasUserObjectByName(object_name))
    _uoi_moose_object.paramError(
        param_name, "The requested UserObject with the name \"", object_name, "\" was not found.");

  return getUserObjectBaseByName(object_name, is_dependency);
}

const UserObject &
UserObjectInterface::getUserObjectBaseByName(const UserObjectName & object_name,
                                             const bool is_dependency) const
{
  if (!hasUserObjectByName(object_name))
    _uoi_moose_object.mooseError(
        "The requested UserObject with the name \"", object_name, "\" was not found.");

  const auto & uo_base_tid0 = _uoi_feproblem.getUserObjectBase(object_name, /* tid = */ 0);
  if (is_dependency)
    addUserObjectDependencyHelper(uo_base_tid0);

  const THREAD_ID tid = uo_base_tid0.needThreadedCopy() ? _uoi_tid : 0;
  return _uoi_feproblem.getUserObjectBase(object_name, tid);
}

const std::string &
UserObjectInterface::userObjectType(const UserObject & uo) const
{
  return uo.type();
}

const std::string &
UserObjectInterface::userObjectName(const UserObject & uo) const
{
  return uo.name();
}
