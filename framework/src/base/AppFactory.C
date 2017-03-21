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

#include "AppFactory.h"
#include "CommandLine.h"
#include "InputParameters.h"

AppFactory AppFactory::_instance = AppFactory();

AppFactory &
AppFactory::instance()
{
  return _instance;
}

AppFactory::~AppFactory() {}

MooseApp *
AppFactory::createApp(std::string app_type, int argc, char ** argv)
{
  auto command_line = std::make_shared<CommandLine>(argc, argv);
  InputParameters app_params = AppFactory::instance().getValidParams(app_type);

  app_params.set<int>("_argc") = argc;
  app_params.set<char **>("_argv") = argv;
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = command_line;

  MooseApp * app = AppFactory::instance().create(app_type, "main", app_params, MPI_COMM_WORLD);
  return app;
}

InputParameters
AppFactory::getValidParams(const std::string & name)
{
  if (_name_to_params_pointer.find(name) == _name_to_params_pointer.end())
    mooseError(std::string("A '") + name + "' is not a registered object\n\n");

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

MooseApp *
AppFactory::create(const std::string & app_type,
                   const std::string & name,
                   InputParameters parameters,
                   MPI_Comm COMM_WORLD_IN)
{
  // Error if the application type is not located
  if (_name_to_build_pointer.find(app_type) == _name_to_build_pointer.end())
    mooseError("Object '" + app_type + "' was not registered.");

  // Take the app_type and add it to the parameters so that it can be retrieved in the Application
  parameters.set<std::string>("_type") = app_type;

  // Check to make sure that all required parameters are supplied
  parameters.checkParams("");

  auto comm = std::make_shared<Parallel::Communicator>(COMM_WORLD_IN);

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

bool
AppFactory::isRegistered(const std::string & app_name) const
{
  return _name_to_params_pointer.find(app_name) != _name_to_params_pointer.end();
}
