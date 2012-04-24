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

#ifndef USERDATAINTERFACE_H
#define USERDATAINTERFACE_H

#include "InputParameters.h"
#include "ParallelUniqueId.h"

class Problem;
class UserData;

/**
 * Interface for objects that need to use user-data objects
 */
class UserDataInterface
{
public:
  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the user data object named in the input file,
   *        but the object calling getUserData only needs to use the name on the
   *        left hand side of the statement "user_data = user_data_name"
   */
  UserDataInterface(InputParameters & params);

  /**
   * Get an user data object with a given name
   * @param name The name of the parameter key of the user data object to retrieve
   * @return The user data object with name associated with the parameter 'name'
   */
  const UserData & getUserData(const std::string & name);

  /**
   * Get a user data object with a given name
   * @param name The name of the user data object to retrieve
   * @return The user data object with name 'name'
   */
  const UserData & getUserDataByName(const std::string & name);

private:
  Problem & _udi_problem;
  /// Thread ID
  THREAD_ID _udi_tid;
  /// Parameters of the object with this interface
  InputParameters _udi_params;
};

#endif //USERDATAFACE_H
