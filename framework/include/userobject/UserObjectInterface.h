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

#ifndef USEROBJECTINTERFACE_H
#define USEROBJECTINTERFACE_H

// MOOSE includes
#include "FEProblemBase.h"
#include "ParallelUniqueId.h"
#include "UserObject.h"

// Forward declarations
class InputParameters;

/**
 * Interface for objects that need to use UserObjects.
 */
class UserObjectInterface
{
public:
  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the user object named in the input file,
   *        but the object calling getUserObject only needs to use the name on the
   *        left hand side of the statement "user_object = user_object_name"
   */
  UserObjectInterface(const MooseObject * moose_object);

  /**
   * Get an user object with a given parameter name
   * @param name The name of the parameter key of the user object to retrieve
   * @return The user object with name associated with the parameter 'name'
   */
  template <class T>
  const T & getUserObject(const std::string & name);

  /**
   * Get an user object with a given name
   * @param name The name of the user object to retrieve
   * @return The user object with the name
   */
  template <class T>
  const T & getUserObjectByName(const std::string & name);

  /**
   * Get an user object with a given parameter name
   * @param name The name of the parameter key of the user object to retrieve
   * @return The user object with name associated with the parameter 'name'
   */
  const UserObject & getUserObjectBase(const std::string & name);

  /**
   * Get an user object with a given name
   * @param name The name of the user object to retrieve
   * @return The user object with the name
   */
  const UserObject & getUserObjectBaseByName(const std::string & name);

private:
  /// Parameters of the object with this interface
  const InputParameters & _uoi_params;

  /// Reference to the FEProblemBase instance
  FEProblemBase & _uoi_feproblem;

  /// Thread ID
  THREAD_ID _uoi_tid;

  /// Check if the user object is a DiscreteElementUserObject
  bool isDiscreteUserObject(const UserObject & uo) const;
};

template <class T>
const T &
UserObjectInterface::getUserObject(const std::string & name)
{
  unsigned int tid = isDiscreteUserObject(getUserObjectBase(name)) ? _uoi_tid : 0;
  return _uoi_feproblem.getUserObject<T>(_uoi_params.get<UserObjectName>(name), tid);
}

template <class T>
const T &
UserObjectInterface::getUserObjectByName(const std::string & name)
{
  unsigned int tid = isDiscreteUserObject(getUserObjectBaseByName(name)) ? _uoi_tid : 0;
  return _uoi_feproblem.getUserObject<T>(name, tid);
}

#endif // USEROBJECTINTERFACE_H
