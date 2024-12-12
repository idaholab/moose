//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseMain.h"
#include "ParallelUniqueId.h"
#include "Parser.h"
#include "AppFactory.h"
#include "CommandLine.h"
#include "InputParameters.h"
#include "MooseApp.h"

#ifdef LIBMESH_HAVE_OPENMP
#include <omp.h>
#endif
#include <regex>

namespace Moose
{

std::shared_ptr<MooseApp>
createMooseApp(const std::string & default_app_type, int argc, char * argv[])
{
  // Parse the command line early in order to determine the application type, from:
  // - the input file, to load and search for Application/type
  // - the --app command line argument
  // - The Application/type= hit command line argument
  CommandLine cl(argc, argv);
  cl.parse();
  auto command_line_params = emptyInputParameters();
  MooseApp::addInputParam(command_line_params);
  MooseApp::addAppParam(command_line_params);
  cl.populateCommandLineParams(command_line_params);

  // Do not allow overriding Application/type= for subapps
  for (const auto & arg : cl.getArguments())
    if (std::regex_match(arg, std::regex("[A-Za-z0-9]*:Application/.*")))
      mooseError(
          "For command line argument '",
          arg,
          "': overriding the application type for MultiApps via command line is not allowed.");

  // Parse the input file; this will set Parser::getAppType() if Application/type= is found
  const auto & input_filenames = command_line_params.get<std::vector<std::string>>("input_file");
  auto parser = std::make_unique<Parser>(input_filenames);
  parser->setAppType(default_app_type);
  if (input_filenames.size())
    parser->parse();

  // Search the command line for either --app or Application/type and let the last one win
  for (const auto & entry : std::as_const(cl).getEntries())
    if (!entry.subapp_name && entry.value &&
        (entry.name == "--app" || entry.name == "Application/type"))
      parser->setAppType(*entry.value);

  const auto & app_type = parser->getAppType();
  if (!AppFactory::instance().isRegistered(app_type))
    mooseError("'", app_type, "' is not a registered application type.");

  // Create an instance of the application and store it in a smart pointer for easy cleanup
  return AppFactory::createAppShared(argc, argv, std::move(parser));
}
}
