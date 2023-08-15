//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AppFactory.h"
#include "CommandLine.h"
#include "InputParameters.h"
#include "MooseApp.h"

AppFactory &
AppFactory::instance()
{
  // We need a naked new here (_not_ a smart pointer or object instance) due to what seems like a
  // bug in clang's static object destruction when using dynamic library loading.
  static AppFactory * instance = nullptr;
  if (!instance)
    instance = new AppFactory;
  return *instance;
}

AppFactory::~AppFactory() {}

InputParameters
AppFactory::getValidParams(const std::string & name)
{
  if (const auto it = _name_to_build_info.find(name); it != _name_to_build_info.end())
    return it->second->buildParameters();

  mooseError(std::string("A '") + name + "' is not a registered object\n\n");
}

MooseAppPtr
AppFactory::createAppShared(const std::string & default_app_type,
                            int argc,
                            char ** argv,
                            MPI_Comm comm_world_in)
{
  auto command_line = std::make_shared<CommandLine>(argc, argv);
  auto which_app_param = emptyInputParameters();
  MooseApp::addAppParam(which_app_param);
  command_line->addCommandLineOptionsFromParams(which_app_param);
  std::string app_type;
  if (!command_line->search("app_to_run", app_type))
    app_type = default_app_type;

  auto app_params = AppFactory::instance().getValidParams(app_type);

  app_params.set<int>("_argc") = argc;
  app_params.set<char **>("_argv") = argv;
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = command_line;

  return AppFactory::instance().createShared(app_type, "main", app_params, comm_world_in);
}

MooseAppPtr
AppFactory::createShared(const std::string & app_type,
                         const std::string & name,
                         InputParameters parameters,
                         MPI_Comm comm_world_in)
{
  // Error if the application type is not located
  const auto it = _name_to_build_info.find(app_type);
  if (it == _name_to_build_info.end())
    mooseError("Object '" + app_type + "' was not registered.");
  auto & build_info = it->second;

  // Take the app_type and add it to the parameters so that it can be retrieved in the Application
  parameters.set<std::string>("_type") = app_type;

  // Check to make sure that all required parameters are supplied
  parameters.checkParams("");

  auto comm = std::make_shared<Parallel::Communicator>(comm_world_in);

  parameters.set<std::shared_ptr<Parallel::Communicator>>("_comm") = comm;
  parameters.set<std::string>("_app_name") = name;

  if (!parameters.isParamValid("_command_line"))
    mooseError("Valid CommandLine object required");

  std::shared_ptr<CommandLine> command_line =
      parameters.get<std::shared_ptr<CommandLine>>("_command_line");
  command_line->addCommandLineOptionsFromParams(parameters);
  command_line->populateInputParams(parameters);

  build_info->_app_creation_count++;

  return build_info->build(parameters);
}

std::size_t
AppFactory::createdAppCount(const std::string & app_type) const
{
  // Error if the application type is not located
  const auto it = _name_to_build_info.find(app_type);
  if (it == _name_to_build_info.end())
    mooseError("AppFactory::createdAppCount(): '", app_type, "' is not a registered app");

  return it->second->_app_creation_count;
}
