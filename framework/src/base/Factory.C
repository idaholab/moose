//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

void
Factory::deprecatedMessage(const std::string obj_name)
{
  // If the object is not deprecated return
  auto iter = _deprecated.find(obj_name);
  if (iter == _deprecated.end())
    return;

  // Build the basic message
  std::ostringstream msg;
  msg << "Deprecated Object: " << obj_name << "\n";

  // Append replacement object, if it exsits
  auto map_iter = _deprecated_with_replace.find(obj_name);
  if (map_iter != _deprecated_with_replace.end())
    msg << "Replaced " << obj_name << " with " << map_iter->second;

  // Produce the error message
  mooseDeprecated(msg.str());
}

void
Factory::reportUnregisteredError(const std::string & obj_name) const
{
  std::ostringstream oss;
  std::set<std::string> paths = _app.getLoadedLibraryPaths();

  oss << "A '" + obj_name + "' is not a registered object.\n";

  if (!paths.empty())
  {
    oss << "\nWe loaded objects from the following libraries and still couldn't find your "
           "object:\n\t";
    std::copy(paths.begin(), paths.end(), infix_ostream_iterator<std::string>(oss, "\n\t"));
    oss << '\n';
  }

  oss << "\nIf you are trying to find this object in a dynamically linked library, make sure that\n"
         "the library can be found either in your \"Problem/library_path\" parameter or in the\n"
         "MOOSE_LIBRARY_PATH environment variable.";

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

void
Factory::associateNameToClass(const std::string & name, const std::string & class_name)
{
  _name_to_class[name] = class_name;
}

std::string
Factory::associatedClassName(const std::string & name) const
{
  auto it = _name_to_class.find(name);
  if (it == _name_to_class.end())
    return "";
  else
    return it->second;
}

void
Factory::deprecateObject(const std::string & name)
{
  _deprecated.insert(name);
}

void
Factory::deprecateObject(const std::string & name, const std::string & replacement)
{
  deprecateObject(name);
  _deprecated_with_replace[name] = replacement;
}

void
Factory::regExecFlag(const ExecFlagType & flag)
{
  _app.addExecFlag(flag);
}
