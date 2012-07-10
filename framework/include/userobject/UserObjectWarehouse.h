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

#ifndef USEROBJECTWAREHOUSE_H
#define USEROBJECTWAREHOUSE_H

#include <vector>
#include <map>

#include "UserObject.h"

/**
 * Warehouse for storing user objects.
 */
class UserObjectWarehouse
{
public:
  UserObjectWarehouse();
  virtual ~UserObjectWarehouse();

  /**
   * Check if there is a user object with given name
   * @param name The name of the object being checked for
   * @return true if the object exists, false otherwise
   */
  bool hasUserObject(const std::string & name);

  /**
   * Get user object by its name
   * @param name Name of the object
   * @return Pointer to the user object
   */
  UserObject * getUserObjectByName(const std::string & name);
  /**
   * Add an user object
   * @param name Name of the object
   * @param user_data Pointer to the object being added
   */
  void addUserObject(const std::string & name, UserObject * user_object);

  /**
   * Get the number of user data objects stored in this warehouse
   * @return The number of objects stored in this warehouse
   */
  unsigned int size() { return _user_objects.size(); }

  /**
   * Get the reference to the list of objects in this warehouse
   * @return The reference to the list of objects in this warehouse
   */
  std::vector<UserObject *> & getObjects() { return _user_objects; }

protected:
  /// storage for user objects
  std::vector<UserObject *> _user_objects;
  /// Map of names to user objects
  std::map<std::string, UserObject *> _name_to_user_objects;
};

#endif // USEROBJECTWAREHOUSE_H
