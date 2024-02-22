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
void
addMainCommandLineParams(InputParameters & params)
{
  params.addCommandLineParam<std::vector<std::string>>(
      "input_file",
      "-i <input files>",
      "Specify one or multiple input files. Multiple files get merged into a single simulation "
      "input.");

  params.addCommandLineParam<std::string>("type",
                                          "--type <app type>",
                                          "Specify the application to run; this must match a "
                                          "registered application type and is case-sensitive");
}

std::shared_ptr<MooseApp>
createMooseApp(const std::string & default_app_name, int argc, char * argv[])
{
  CommandLine cl(argc, argv);
  cl.parse();
  auto command_line_params = emptyInputParameters();
  addMainCommandLineParams(command_line_params);
  cl.populateCommandLineParams(command_line_params);

  const auto & input_filenames = command_line_params.get<std::vector<std::string>>("input_file");
  const auto & cl_app_type = command_line_params.get<std::string>("type");

  // loop over all the command line arguments and error out when the user uses Application block for
  // subapps
  auto cli_args = cl.getArguments();
  if (std::find_if(cli_args.begin(),
                   cli_args.end(),
                   [&](auto & arg) {
                     return std::regex_match(arg, std::regex("[A-Za-z0-9]*:Application/.*"));
                   }) != cli_args.end())
    mooseError("Using the CommandLine option to overwite [Application] block is not supported for "
               "sub_apps");

  auto parser = std::make_unique<Parser>(input_filenames);
  if (input_filenames.size())
    parser->parse();

  // Check whether the application name given in [Application] block is registered or not
  if (!cl_app_type.empty())
    parser->setAppType(cl_app_type);

  auto app_type = parser->getAppType();
  if (!app_type.empty())
    if (!AppFactory::instance().isRegistered(app_type))
      mooseError("'", app_type, "' is not a registered application name.\n");

  // Create an instance of the application and store it in a smart pointer for easy cleanup
  return AppFactory::createAppShared(default_app_name, argc, argv, std::move(parser));
}
}
