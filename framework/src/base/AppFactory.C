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
  static AppFactory instance;
  return instance;
}

AppFactory::~AppFactory() {}

InputParameters
AppFactory::getValidParams(const std::string & name)
{
  if (_name_to_params_pointer.find(name) == _name_to_params_pointer.end())
    mooseError(std::string("A '") + name + "' is not a registered object\n\n");

  InputParameters params = _name_to_params_pointer[name]();
  return params;
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
  if (_name_to_build_pointer.find(app_type) == _name_to_build_pointer.end())
    mooseError("Object '" + app_type + "' was not registered.");

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

  return (*_name_to_build_pointer[app_type])(parameters);
}
