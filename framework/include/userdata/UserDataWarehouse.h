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

#ifndef USERDATAWAREHOUSE_H
#define USERDATAWAREHOUSE_H

#include <vector>
#include <map>

#include "UserData.h"

/**
 * Warehouse for storing user-data objects.
 */
class UserDataWarehouse
{
public:
  UserDataWarehouse();
  virtual ~UserDataWarehouse();

  /**
   * Get user-data object by its name
   * @param name Name of the object
   * @return Pointer to the user data object
   */
  UserData * getUserDataByName(const std::string & name);
  /**
   * Add an user-data object
   * @param name Name of the object
   * @param user_data Pointer to the object being added
   */
  void addUserData(const std::string & name, UserData * user_data);

protected:
  /// storage for user data
  std::vector<UserData *> _user_data;
  /// Map of names to user data
  std::map<std::string, UserData *> _name_to_user_data;
};

#endif // USERDATAWAREHOUSE_H
