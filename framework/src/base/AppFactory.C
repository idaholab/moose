//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

const std::string AppFactory::main_app_name = "main";

InputParameters
AppFactory::getValidParams(const std::string & name)
{
  if (const auto it = _name_to_build_info.find(name); it != _name_to_build_info.end())
    return it->second->buildParameters();

  mooseError(std::string("A '") + name + "' is not a registered object\n\n");
}

const InputParameters &
AppFactory::getAppParams(const InputParameters & params) const
{
  const auto id = getAppParamsID(params);
  if (const auto it = _input_parameters.find(id); it != _input_parameters.end())
    return *it->second;
  mooseError("AppFactory::getAppParams(): Parameters for application with ID ", id, " not found");
}

void
AppFactory::clearAppParams(const InputParameters & params, const ClearAppParamsKey)
{
  const auto id = getAppParamsID(params);
  if (const auto it = _input_parameters.find(id); it != _input_parameters.end())
    _input_parameters.erase(it);
  else
    mooseError(
        "AppFactory::clearAppParams(): Parameters for application with ID ", id, " not found");
}

std::unique_ptr<MooseApp>
AppFactory::create(std::unique_ptr<Parser> parser, std::unique_ptr<CommandLine> command_line)
{
  mooseAssert(parser, "Not set");
  mooseAssert(parser->getAppType().size(), "App type not set");
  mooseAssert(parser->queryRoot(), "Has not parsed");
  mooseAssert(command_line, "Not set");
  mooseAssert(command_line->hasParsed(), "Has not parsed");

  const std::string app_type = parser->getAppType();

  auto app_params = AppFactory::instance().getValidParams(parser->getAppType());
  app_params.set<std::shared_ptr<Parser>>("_parser") = std::move(parser);
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = std::move(command_line);

  return AppFactory::instance().create(app_type, main_app_name, app_params, MPI_COMM_WORLD);
}

std::shared_ptr<MooseApp>
AppFactory::createAppShared(const std::string & default_app_type,
                            int argc,
                            char ** argv,
                            MPI_Comm comm_world_in)
{
  mooseDeprecated("Please update your main.C to adapt new main function in MOOSE framework, "
                  "see'test/src/main.C in MOOSE as an example of moose::main()'. ");

  // Populate the -i and --app parameters early
  auto command_line_params = emptyInputParameters();
  MooseApp::addInputParam(command_line_params);
  MooseApp::addAppParam(command_line_params);
  {
    CommandLine pre_command_line(argc, argv);
    pre_command_line.parse();
    pre_command_line.populateCommandLineParams(command_line_params);
  }
  const auto & input_filenames = command_line_params.get<std::vector<std::string>>("input_file");
  std::string app_type = command_line_params.get<std::string>("app_to_run");

  auto command_line = std::make_unique<CommandLine>(argc, argv);
  command_line->parse();

  auto parser = std::make_unique<Parser>(input_filenames);
  parser->setCommandLineParams(command_line->buildHitParams());
  parser->parse();

  if (app_type.empty())
    app_type = default_app_type;
  else
    mooseDeprecated("Please use [Application] block to specify application type, '--app <AppName>' "
                    "is deprecated and will be removed in a future release.");

  parser->setAppType(app_type);

  auto app_params = AppFactory::instance().getValidParams(app_type);
  app_params.set<std::shared_ptr<Parser>>("_parser") = std::move(parser);
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = std::move(command_line);

  return AppFactory::instance().create(app_type, "main", app_params, comm_world_in);
}

std::unique_ptr<MooseApp>
AppFactory::create(const std::string & app_type,
                   const std::string & name,
                   InputParameters parameters,
                   MPI_Comm comm_world_in)
{
  // Error if the application type is not located
  const auto it = _name_to_build_info.find(app_type);
  if (it == _name_to_build_info.end())
    mooseError("AppFactory::Create(): Application '" + app_type + "' was not registered");
  auto & build_info = it->second;

  auto comm = std::make_shared<Parallel::Communicator>(comm_world_in);

  // Take the app_type and add it to the parameters so that it can be retrieved in the Application
  parameters.set<std::string>(MooseBase::type_param) = app_type;
  parameters.set<std::string>(MooseBase::name_param) = name;
  parameters.set<std::string>(MooseBase::unique_name_param) = "Application/" + name;
  parameters.set<std::shared_ptr<Parallel::Communicator>>("_comm") = comm;
  parameters.set<std::string>("_app_name") = name;

  auto parser = parameters.get<std::shared_ptr<Parser>>("_parser");
  mooseAssert(parser, "Parser not valid");
  mooseAssert(parser->queryRoot() && parser->queryCommandLineRoot(), "Parser has not parsed");

  auto command_line = parameters.get<std::shared_ptr<CommandLine>>("_command_line");
  mooseAssert(command_line, "Command line not valid");
  mooseAssert(command_line->hasParsed(), "Command line has not parsed");
  command_line->populateCommandLineParams(parameters);

  // Historically we decided to non-const copy construct all application parameters. In
  // order to get around that while apps are fixed (by taking a const reference instead),
  // we store the app params here and the MooseApp constructor will query the InputParameters
  // owned by ths factory instead of the ones that are passed to it (likely a const ref to a
  // copy of the derived app's parmeters)
  const auto & params = storeAppParams(parameters);

  build_info->_app_creation_count++;

  return build_info->build(params);
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

const InputParameters &
AppFactory::storeAppParams(InputParameters & params)
{
  const std::size_t next_id =
      _input_parameters.size() ? (std::prev(_input_parameters.end())->first + 1) : 0;
  params.addPrivateParam<std::size_t>("_app_params_id", next_id);
  const auto it_inserted_pair =
      _input_parameters.emplace(next_id, std::make_unique<InputParameters>(params));
  mooseAssert(it_inserted_pair.second, "Already exists");
  auto & stored_params = *it_inserted_pair.first->second;
  stored_params.finalize("");
  return stored_params;
}

std::size_t
AppFactory::getAppParamsID(const InputParameters & params) const
{
  if (!params.have_parameter<std::size_t>("_app_params_id"))
    mooseError("AppFactory::getAppParamsID(): Invalid application parameters (missing "
               "'_app_params_id')");
  return params.get<std::size_t>("_app_params_id");
}
