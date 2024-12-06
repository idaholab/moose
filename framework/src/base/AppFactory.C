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
#include "Parser.h"
#include "MooseMain.h"

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
AppFactory::createAppShared(int argc, char ** argv, std::unique_ptr<Parser> parser)
{
  mooseAssert(parser, "Not set");
  mooseAssert(parser->getAppType().size(), "App type not set");
  const std::string app_type = parser->getAppType();

  auto command_line = std::make_unique<CommandLine>(argc, argv);
  command_line->parse();

  auto app_params = AppFactory::instance().getValidParams(parser->getAppType());
  app_params.set<int>("_argc") = argc;
  app_params.set<char **>("_argv") = argv;
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = std::move(command_line);
  app_params.set<std::shared_ptr<Parser>>("_parser") = std::move(parser);

  return AppFactory::instance().createShared(app_type, "main", app_params, MPI_COMM_WORLD);
}

MooseAppPtr
AppFactory::createAppShared(const std::string & default_app_type,
                            int argc,
                            char ** argv,
                            MPI_Comm comm_world_in)
{
  mooseDeprecated("Please update your main.C to adapt new main function in MOOSE framework, "
                  "see'test/src/main.C in MOOSE as an example of moose::main()'. ");

  auto command_line_params = emptyInputParameters();
  MooseApp::addInputParam(command_line_params);
  MooseApp::addAppParam(command_line_params);

  {
    CommandLine pre_command_line(argc, argv);
    pre_command_line.parse();
    pre_command_line.populateCommandLineParams(command_line_params);
  }

  const auto & input_filenames = command_line_params.get<std::vector<std::string>>("input_file");
  auto parser = std::make_unique<Parser>(input_filenames);
  if (input_filenames.size())
    parser->parse();

  std::string app_type = command_line_params.get<std::string>("app_to_run");
  if (app_type.empty())
    app_type = default_app_type;
  else
    mooseDeprecated("Please use [Application] block to specify application type, '--app <AppName>' "
                    "is deprecated and will be removed in a future release.");

  parser->setAppType(app_type);
  auto app_params = AppFactory::instance().getValidParams(app_type);

  app_params.set<int>("_argc") = argc;
  app_params.set<char **>("_argv") = argv;

  auto command_line = std::make_unique<CommandLine>(argc, argv);
  command_line->parse();
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = std::move(command_line);

  // Take the front parser and add it to the parameters so that it can be retrieved in the
  // Application
  app_params.set<std::shared_ptr<Parser>>("_parser") = std::move(parser);

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
  parameters.finalize("");

  auto comm = std::make_shared<Parallel::Communicator>(comm_world_in);

  parameters.set<std::shared_ptr<Parallel::Communicator>>("_comm") = comm;
  parameters.set<std::string>("_app_name") = name;

  if (!parameters.isParamValid("_command_line"))
    mooseError("Valid CommandLine object required");

  std::shared_ptr<CommandLine> command_line =
      parameters.get<std::shared_ptr<CommandLine>>("_command_line");
  mooseAssert(command_line->hasParsed(), "Should have been parsed");

  command_line->populateCommandLineParams(parameters);

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
