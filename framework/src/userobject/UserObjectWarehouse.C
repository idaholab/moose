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

#include "UserObjectWarehouse.h"
#include "Moose.h"


UserObjectWarehouse::UserObjectWarehouse()
{
}

UserObjectWarehouse::~UserObjectWarehouse()
{
  for (std::vector<UserObject *>::iterator it = _user_objects.begin(); it != _user_objects.end(); ++it)
  {
    UserObject * uo = *it;
    uo->destroy();
    delete uo;
  }
}

bool
UserObjectWarehouse::hasUserObject(const std::string & name)
{
  std::map<std::string, UserObject *>::iterator it = _name_to_user_objects.find(name);
  return (it != _name_to_user_objects.end());
}

void
UserObjectWarehouse::addUserObject(const std::string & name, UserObject * user_object)
{
  _user_objects.push_back(user_object);
  _name_to_user_objects[name] = user_object;
}

UserObject *
UserObjectWarehouse::getUserObjectByName(const std::string & name)
{
  std::map<std::string, UserObject *>::iterator it = _name_to_user_objects.find(name);
  if (it == _name_to_user_objects.end())
    mooseError("Could not find user object with name '" << name << "'");
  return it->second;
}
