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

#include "Factory.h"
#include "MooseApp.h"

Factory::Factory(MooseApp & app):
    _app(app)
{
}

Factory::~Factory()
{
}

InputParameters
Factory::getValidParams(const std::string & obj_name)
{
  // Check if the object is registered
  if (_name_to_params_pointer.find(obj_name) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + obj_name + "' is not a registered object\n\n");

  // Print out deprecated message, if it exists
  deprecatedMessage(obj_name);

  // Return the parameters
  InputParameters params = _name_to_params_pointer[obj_name]();
  params.addPrivateParam("_moose_app", &_app);
  return params;
}

MooseObject *
Factory::create(const std::string & obj_name, const std::string & name, InputParameters parameters)
{
  // Check if the object is registered
  if (_name_to_build_pointer.find(obj_name) == _name_to_build_pointer.end())
    mooseError("Object '" + obj_name + "' was not registered.");

  // Print out deprecated message, if it exists
  deprecatedMessage(obj_name);

  // Check to make sure that all required parameters are supplied
  parameters.checkParams(name);
  return (*_name_to_build_pointer[obj_name])(name, parameters);
}

time_t Factory::parseTime(const std::string t_str)
{
  // The string must be a certain length to be valid
  if (t_str.size() != 16)
    mooseError("The deprected time not formatted correctly; it must be given as mm/dd/yyyy HH:MM");

  // Store the time, the time must be specified as: mm/dd/yyyy HH:MM
  time_t t_end;
  struct tm * t_end_info;
  time(&t_end);
  t_end_info = localtime(&t_end);
  t_end_info->tm_mon  = std::atoi(t_str.substr(0,2).c_str())-1;
  t_end_info->tm_mday = std::atoi(t_str.substr(3,2).c_str());
  t_end_info->tm_year = std::atoi(t_str.substr(6,4).c_str())-1900;
  t_end_info->tm_hour = std::atoi(t_str.substr(11,2).c_str());
  t_end_info->tm_min  = std::atoi(t_str.substr(15,2).c_str());
  t_end_info->tm_sec  = 0;
  t_end = mktime(t_end_info);
  return t_end;
}

void Factory::deprecatedMessage(const std::string obj_name)
{
  // If the object is not deprecated return
  if (_deprecated_time.find(obj_name) == _deprecated_time.end() )
    return;

  // Get the current time
  time_t now;
  time(&now);

  // Get the stop time
  time_t t_end =  _deprecated_time[obj_name];

  // Message storage
  std::ostringstream msg;

  // Expired object
  if( now > t_end )
  {
    msg << "***** Invalid Object: " << obj_name << " *****\n";
    msg << "Expired on " << ctime(&t_end);

    // Append replacement object, if it exsits
    if (_deprecated_name.find(obj_name) != _deprecated_name.end())
      msg << "Upadate your application using the '" << _deprecated_name[obj_name] << "' object";

    // Produce the error message
    mooseError(msg.str());
  }

  // Expiring object
  else
  {
    // Build the basic message
    msg << "Deprecated Object: " << obj_name << "\n";
    msg << "This object will be removed on " << ctime(&t_end);

    // Append replacement object, if it exsits
    if (_deprecated_name.find(obj_name) != _deprecated_name.end())
      msg << "Replaced " << obj_name << " with " <<  _deprecated_name[obj_name];

    // Produce the error message
    mooseDoOnce(mooseWarning(msg.str()));
  }
}
