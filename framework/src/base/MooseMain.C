//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseMain.h"
#include "Parser.h"
#include "CommandLine.h"
#include "InputParameters.h"
#include "MooseApp.h"
#include "AppBuilder.h"
#include "AppFactory.h"

#ifdef LIBMESH_HAVE_OPENMP
#include <omp.h>
#endif
#include <regex>

namespace Moose
{
std::shared_ptr<MooseApp>
createMooseApp(const std::string & default_app_name, int argc, char * argv[])
{
  // Setup a command line to search for the input file
  CommandLine command_line(argc, argv);
  command_line.parse();

  // Parse the -i command line option
  auto input_param = emptyInputParameters();
  MooseApp::addInputFileParam(input_param); // adds -i
  command_line.populateCommandLineParams(input_param);
  std::vector<std::string> input_filenames =
      input_param.get<std::vector<std::string>>("input_file");

  // Parse the input files (if any)
  auto parser = std::make_unique<Parser>(input_filenames);
  parser->parse();

  // Build the application's parameters
  auto app_builder = std::make_unique<Moose::AppBuilder>(std::move(parser));
  auto params = app_builder->buildParams(default_app_name, "main", argc, argv, MPI_COMM_WORLD);

  // Build the application
  return AppFactory::instance().createShared(params);
}
}
