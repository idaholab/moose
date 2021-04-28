//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblemBase.h"

// Forward declarations
class UserObject;

/**
 * Interface for objects that need to use UserObjects.
 */
class UserObjectInterface
{
public:
  static InputParameters validParams();

  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the user object named in the input file,
   *        but the object calling getUserObject only needs to use the name on the
   *        left hand side of the statement "user_object = user_object_name"
   */
  UserObjectInterface(const MooseObject * moose_object);

  /**
   * @return The name of the user object associated with the parameter \p param_name
   */
  UserObjectName getUserObjectName(const std::string & param_name) const;

  /**
   * @return Whether or not a UserObject exists with the name given by the parameter \p param_name.
   */
  ///@{
  bool hasUserObject(const std::string & param_name) const;
  template <class T>
  bool hasUserObject(const std::string & param_name) const;
  ///@}

  /*
   * @return Whether or not a UserObject exists with the name \p object_name.
   */
  ///@{
  bool hasUserObjectByName(const UserObjectName & object_name) const;
  template <class T>
  bool hasUserObjectByName(const UserObjectName & object_name) const;
  ///@}

  /**
   * Get an user object with a given parameter \p param_name
   * @param param_name The name of the parameter key of the user object to retrieve
   * @return The user object with name associated with the parameter \p param_name
   */
  template <class T>
  const T & getUserObject(const std::string & param_name) const;

  /**
   * Get an user object with the name \p object_name
   * @param object_name The name of the user object to retrieve
   * @return The user object with the name \p object_name
   */
  template <class T>
  const T & getUserObjectByName(const UserObjectName & object_name) const;

  /**
   * Get an user object with a given parameter \p param_name
   * @param param_name The name of the parameter key of the user object to retrieve
   * @return The user object with name associated with the parameter \p param_name
   */
  const UserObject & getUserObjectBase(const std::string & param_name) const;

  /**
   * Get an user object with the name \p object_name
   * @param object_name The name of the user object to retrieve
   * @return The user object with the name \p object_name
   */
  const UserObject & getUserObjectBaseByName(const UserObjectName & object_name) const;

protected:
  /**
   * Helper for deriving classes to override to add dependencies when a UserObject
   * is requested.
   */
  virtual void addUserObjectDependencyHelper(const UserObject & /* uo */) const {}

private:
  /**
   * Internal helper that casts the UserObject \p uo_base to the reqested type. Exits with
   * a useful error if the casting failed. If the parameter \p param_name is provided and
   * is valid, a paramError() will be used instead.
   */
  template <class T>
  const T & castUserObject(const UserObject & uo_base, const std::string & param_name = "") const;

  /// Gets a UserObject's type; avoids including UserObject.h in the UserObjectInterface
  const std::string & userObjectType(const UserObject & uo) const;
  /// Gets a UserObject's name; avoids including UserObject.h in the UserObjectInterface
  const std::string & userObjectName(const UserObject & uo) const;

  /// Moose object using the interface
  const MooseObject & _uoi_moose_object;

  /// Reference to the FEProblemBase instance
  const FEProblemBase & _uoi_feproblem;

  /// Thread ID
  const THREAD_ID _uoi_tid;
};

template <class T>
const T &
UserObjectInterface::castUserObject(const UserObject & uo_base,
                                    const std::string & param_name /* = "" */) const
{
  const T * uo = dynamic_cast<const T *>(&uo_base);

  if (!uo)
  {
    std::stringstream oss;
    oss << "The provided UserObject \"" << userObjectName(uo_base) << "\" of type "
        << userObjectType(uo_base)
        << " is not derived from the required type.\n\nThe UserObject must derive from "
        << MooseUtils::prettyCppType<T>() << ".";

    if (_uoi_moose_object.parameters().isParamValid(param_name))
      _uoi_moose_object.paramError(param_name, oss.str());
    else
      _uoi_moose_object.mooseError(oss.str());
  }

  return *uo;
}

template <class T>
const T &
UserObjectInterface::getUserObject(const std::string & param_name) const
{
  return castUserObject<T>(getUserObjectBase(param_name), param_name);
}

template <class T>
const T &
UserObjectInterface::getUserObjectByName(const UserObjectName & object_name) const
{
  return castUserObject<T>(getUserObjectBaseByName(object_name));
}

template <class T>
bool
UserObjectInterface::hasUserObject(const std::string & param_name) const
{
  return hasUserObjectByName(getUserObjectName(param_name));
}

template <class T>
bool
UserObjectInterface::hasUserObjectByName(const UserObjectName & object_name) const
{
  if (!hasUserObjectByName(object_name))
    return false;
  return dynamic_cast<const T *>(&_uoi_feproblem.getUserObjectBase(object_name));
}
