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
#include "InfixIterator.h"
#include "InputParameterWarehouse.h"
// Just for testing...
#include "Diffusion.h"

Factory::Factory(MooseApp & app) : _app(app) {}

Factory::~Factory() {}

InputParameters
Factory::getValidParams(const std::string & obj_name)
{
  std::map<std::string, paramsPtr>::iterator it = _name_to_params_pointer.find(obj_name);

  // Check if the object is registered
  if (it == _name_to_params_pointer.end())
    reportUnregisteredError(obj_name);

  // Print out deprecated message, if it exists
  deprecatedMessage(obj_name);

  // Return the parameters
  paramsPtr & func = it->second;
  InputParameters params = (*func)();
  params.addPrivateParam("_moose_app", &_app);

  return params;
}

MooseObjectPtr
Factory::create(const std::string & obj_name,
                const std::string & name,
                InputParameters parameters,
                THREAD_ID tid /* =0 */,
                bool print_deprecated /* =true */)
{
  if (print_deprecated)
    mooseDeprecated("Factory::create() is deprecated, please use Factory::create<T>() instead");

  // Pointer to the object constructor
  std::map<std::string, buildPtr>::iterator it = _name_to_build_pointer.find(obj_name);

  // Check if the object is registered
  if (it == _name_to_build_pointer.end())
    reportUnregisteredError(obj_name);

  // Print out deprecated message, if it exists
  deprecatedMessage(obj_name);

  // Create the actual parameters object that the object will reference
  InputParameters & params =
      _app.getInputParameterWarehouse().addInputParameters(name, parameters, tid);

  // Check to make sure that all required parameters are supplied
  params.checkParams(name);

  // register type name as constructed
  _constructed_types.insert(obj_name);

  // Actually call the function pointer.  You can do this in one line,
  // but it's a bit more obvious what's happening if you do it in two...
  buildPtr & func = it->second;
  return (*func)(params);
}

void
Factory::restrictRegisterableObjects(const std::vector<std::string> & names)
{
  _registerable_objects.insert(names.begin(), names.end());
}

time_t
Factory::parseTime(const std::string t_str)
{
  // The string must be a certain length to be valid
  if (t_str.size() != 16)
    mooseError("The deprecated time not formatted correctly; it must be given as mm/dd/yyyy HH:MM");

  // Store the time, the time must be specified as: mm/dd/yyyy HH:MM
  time_t t_end;
  struct tm * t_end_info;
  time(&t_end);
  t_end_info = localtime(&t_end);
  t_end_info->tm_mon = std::atoi(t_str.substr(0, 2).c_str()) - 1;
  t_end_info->tm_mday = std::atoi(t_str.substr(3, 2).c_str());
  t_end_info->tm_year = std::atoi(t_str.substr(6, 4).c_str()) - 1900;
  t_end_info->tm_hour = std::atoi(t_str.substr(11, 2).c_str()) + 1;
  t_end_info->tm_min = std::atoi(t_str.substr(14, 2).c_str());
  t_end_info->tm_sec = 0;
  t_end = mktime(t_end_info);
  return t_end;
}

void
Factory::deprecatedMessage(const std::string obj_name)
{
  std::map<std::string, time_t>::iterator time_it = _deprecated_time.find(obj_name);

  // If the object is not deprecated return
  if (time_it == _deprecated_time.end())
    return;

  // Get the current time
  time_t now;
  time(&now);

  // Get the stop time
  time_t t_end = time_it->second;

  // Message storage
  std::ostringstream msg;

  std::map<std::string, std::string>::iterator name_it = _deprecated_name.find(obj_name);

  // Expired object
  if (now > t_end)
  {
    msg << "***** Invalid Object: " << obj_name << " *****\n";
    msg << "Expired on " << ctime(&t_end);

    // Append replacement object, if it exsits
    if (name_it != _deprecated_name.end())
      msg << "Update your application using the '" << name_it->second << "' object";

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
    if (name_it != _deprecated_name.end())
      msg << "Replaced " << obj_name << " with " << name_it->second;

    // Produce the error message
    mooseDeprecated(msg.str());
  }
}

void
Factory::reportUnregisteredError(const std::string & obj_name) const
{
  std::ostringstream oss;
  std::set<std::string> paths = _app.getLoadedLibraryPaths();

  oss << "A '" + obj_name + "' is not a registered object.\n"
      << "\nWe loaded objects from the following libraries and still couldn't find your "
         "object:\n\t";
  std::copy(paths.begin(), paths.end(), infix_ostream_iterator<std::string>(oss, "\n\t"));
  if (paths.empty())
    oss << "(NONE)\n";
  oss << "\n\nMake sure you have compiled the library and either set the \"library_path\" variable "
      << "in your input file or exported \"MOOSE_LIBRARY_PATH\".";

  mooseError(oss.str());
}

std::vector<std::string>
Factory::getConstructedObjects() const
{
  std::vector<std::string> list;
  for (const auto & name : _constructed_types)
    list.push_back(name);
  return list;
}

FileLineInfo
Factory::getLineInfo(const std::string & name) const
{
  return _name_to_line.getInfo(name);
}
