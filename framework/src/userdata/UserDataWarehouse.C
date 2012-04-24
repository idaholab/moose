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

#include "UserDataWarehouse.h"
#include "Moose.h"


UserDataWarehouse::UserDataWarehouse()
{
}

UserDataWarehouse::~UserDataWarehouse()
{
  for (std::vector<UserData *>::iterator it = _user_data.begin(); it != _user_data.end(); ++it)
    delete (*it);
}

void
UserDataWarehouse::addUserData(const std::string & name, UserData * user_data)
{
  _user_data.push_back(user_data);
  _name_to_user_data[name] = user_data;
}

UserData *
UserDataWarehouse::getUserDataByName(const std::string & name)
{
  std::map<std::string, UserData *>::iterator it = _name_to_user_data.find(name);
  if (it == _name_to_user_data.end())
    mooseError("Could not find user-data object with name '" << name << "'");
  return it->second;
}
