//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"
#include "MooseUtils.h"

// Forward declarations
class UserObject;
class FEProblemBase;
class MooseObject;

/**
 * Interface for objects that need to use UserObjects.
 */
class UserObjectInterface
{
public:
  static InputParameters validParams();

  UserObjectInterface(const MooseObject * moose_object, bool initialize = true);

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
   * @param is_dependency Whether the user object we are retrieving should be viewed as a
   * dependency, e.g. whether the retrieved user object should be sorted and executed before this
   * object (if we are a user object)
   * @return The user object with name associated with the parameter \p param_name
   */
  template <class T>
  const T & getUserObject(const std::string & param_name, bool is_dependency = true) const;

  /**
   * Get an user object with the name \p object_name
   * @param object_name The name of the user object to retrieve
   * @param is_dependency Whether the user object we are retrieving should be viewed as a
   * dependency, e.g. whether the retrieved user object should be sorted and executed before this
   * object (if we are a user object)
   * @return The user object with the name \p object_name
   */
  template <class T>
  const T & getUserObjectByName(const UserObjectName & object_name,
                                bool is_dependency = true) const;

  /**
   * Get an user object with a given parameter \p param_name
   * @param param_name The name of the parameter key of the user object to retrieve
   * @param is_dependency Whether the user object we are retrieving should be viewed as a
   * dependency, e.g. whether the retrieved user object should be sorted and executed before this
   * object (if we are a user object)
   * @return The user object with name associated with the parameter \p param_name
   */
  const UserObject & getUserObjectBase(const std::string & param_name,
                                       bool is_dependency = true) const;

  /**
   * Get an user object with the name \p object_name
   * @param object_name The name of the user object to retrieve
   * @param is_dependency Whether the user object we are retrieving should be viewed as a
   * dependency, e.g. whether the retrieved user object should be sorted and executed before this
   * object (if we are a user object)
   * @return The user object with the name \p object_name
   */
  const UserObject & getUserObjectBaseByName(const UserObjectName & object_name,
                                             bool is_dependency = true) const;

protected:
  /**
   * Helper for deriving classes to override to add dependencies when a UserObject
   * is requested.
   */
  virtual void addUserObjectDependencyHelper(const UserObject & /* uo */) const {}

private:
  /**
   * Go directly to the FEProblem for the requested \p UserObject
   */
  const UserObject & getUserObjectFromFEProblem(const UserObjectName & object_name) const;

  /**
   * Internal helper that casts the UserObject \p uo_base to the requested type. Exits with
   * a useful error if the casting failed. If the parameter \p param_name is provided and
   * is valid, a paramError() will be used instead.
   */
  template <class T>
  const T & castUserObject(const UserObject & uo_base, const std::string & param_name = "") const;

  /**
   * emit an error for the given parameter
   */
  void mooseObjectError(const std::string & param_name, std::stringstream & oss) const;

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

    mooseObjectError(param_name, oss);
  }

  return *uo;
}

template <class T>
const T &
UserObjectInterface::getUserObject(const std::string & param_name, const bool is_dependency) const
{
  return castUserObject<T>(getUserObjectBase(param_name, is_dependency), param_name);
}

template <class T>
const T &
UserObjectInterface::getUserObjectByName(const UserObjectName & object_name,
                                         const bool is_dependency) const
{
  return castUserObject<T>(getUserObjectBaseByName(object_name, is_dependency));
}

template <class T>
bool
UserObjectInterface::hasUserObject(const std::string & param_name) const
{
  return hasUserObjectByName<T>(getUserObjectName(param_name));
}

template <class T>
bool
UserObjectInterface::hasUserObjectByName(const UserObjectName & object_name) const
{
  if (!hasUserObjectByName(object_name))
    return false;
  return dynamic_cast<const T *>(&getUserObjectFromFEProblem(object_name));
}
