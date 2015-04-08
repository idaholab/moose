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

AppFactory AppFactory::_instance = AppFactory();

AppFactory &AppFactory::instance()
{
  return _instance;
}

AppFactory::~AppFactory()
{
}

MooseApp *
AppFactory::createApp(std::string app_type, int argc, char ** argv)
{
  MooseSharedPointer<CommandLine> command_line(new CommandLine(argc, argv));
  InputParameters app_params = AppFactory::instance().getValidParams(app_type);

  app_params.set<int>("_argc") = argc;
  app_params.set<char**>("_argv") = argv;
  app_params.set<MooseSharedPointer<CommandLine> >("_command_line") = command_line;

  MooseApp * app = AppFactory::instance().create(app_type, "main", app_params, MPI_COMM_WORLD);
  return app;
}

InputParameters
AppFactory::getValidParams(const std::string & name)
{
  if (_name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + name + "' is not a registered object\n\n");

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

MooseApp *
AppFactory::create(const std::string & obj_name, const std::string & name, InputParameters parameters, MPI_Comm COMM_WORLD_IN)
{
  if (_name_to_build_pointer.find(obj_name) == _name_to_build_pointer.end())
    mooseError("Object '" + obj_name + "' was not registered.");

  // Check to make sure that all required parameters are supplied
  parameters.checkParams("");

  MooseSharedPointer<Parallel::Communicator> comm(new Parallel::Communicator(COMM_WORLD_IN));

  parameters.set<MooseSharedPointer<Parallel::Communicator> >("_comm") = comm;

  if (!parameters.isParamValid("_command_line"))
    mooseError("Valid CommandLine object required");

  MooseSharedPointer<CommandLine> command_line = parameters.get<MooseSharedPointer<CommandLine> >("_command_line");
  command_line->addCommandLineOptionsFromParams(parameters);
  command_line->populateInputParams(parameters);

  return (*_name_to_build_pointer[obj_name])(name, parameters);
}

bool
AppFactory::isRegistered(const std::string & app_name) const
{
  return _name_to_params_pointer.find(app_name) != _name_to_params_pointer.end();
}
