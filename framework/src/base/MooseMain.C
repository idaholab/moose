//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

std::unique_ptr<MooseApp>
createMooseApp(const std::string & default_app_type, int argc, char * argv[])
{
  auto & app_factory = AppFactory::instance();
  if (!app_factory.isRegistered(default_app_type))
    mooseError("createMooseApp: The default app type '" + default_app_type +
               "' is not a registered application type");

  // Parse the command line early in order to determine the application type, from:
  // - the input file, to load and search for Application/type
  // - the --app command line argument
  // - The Application/type= hit command line argument
  auto command_line_params = app_factory.getValidParams(default_app_type);
  {
    CommandLine cl(argc, argv);
    cl.parse();
    cl.populateCommandLineParams(command_line_params, nullptr, std::set<std::string>{"input_file"});

    // Do not allow overriding Application/type= for subapps
    for (const auto & arg : cl.getArguments())
      if (std::regex_match(arg, std::regex("^[A-Za-z0-9]+:Application/type=.*")))
        mooseError(
            "For command line argument '",
            arg,
            "': overriding the application type for MultiApps via command line is not allowed.");
  }
  const auto & input_filenames = command_line_params.get<std::vector<std::string>>("input_file");

  // Parse command line arguments so that we can get the "--app" entry (if any) and the HIT
  // command line arguments for the Parser
  auto command_line = std::make_unique<CommandLine>(argc, argv);
  command_line->parse();

  // Setup the parser with the input and the HIT parameters from the command line. The parse
  // will also look for "Application/type=" in input to specify the application type
  auto parser = std::make_unique<Parser>(input_filenames);
  parser->setAppType(default_app_type, nullptr);
  parser->setCommandLineParams(command_line->buildHitParams());
  parser->parse();

  // Search the command line for either --app or Application/type and let the last one win
  for (const auto & entry : std::as_const(*command_line).getEntries())
    if (!entry.subapp_name && entry.value)
    {
      if (entry.name == "--app")
        parser->setAppType(*entry.value, &parser->getCommandLineRoot());
      else if (entry.name == "Application/type")
        parser->setAppType(*entry.value, parser->getCommandLineRoot().find("Application/type"));
    }

  const auto & [app_type, node] = *parser->getAppType();
  if (app_type != default_app_type && !app_factory.isRegistered(app_type))
  {
    auto error = "'" + app_type + "' is not a registered application type";
    if (node)
      if (const auto hit_prefix = Moose::hitMessagePrefix(*node, true))
        error = *hit_prefix + "\n" + error;
    mooseError(error);
  }

  // Create an instance of the application and store it in a smart pointer for easy cleanup
  return AppFactory::create(std::move(parser), std::move(command_line));
}
}
