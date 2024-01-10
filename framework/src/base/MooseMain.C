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

namespace Moose
{
void
addMainCommandLineParams(InputParameters & params)
{
  params.addCommandLineParam<std::vector<std::string>>(
      "input_file",
      "-i <input_files>",
      "Specify one or multiple input files. Multiple files get merged into a single simulation "
      "input.");
}

std::shared_ptr<MooseApp>
createMooseApp(const std::string & default_app_name, int argc, char * argv[])
{
  auto command_line = std::make_shared<CommandLine>(argc, argv);

  {
    auto input_param = emptyInputParameters();
    addMainCommandLineParams(input_param);
    command_line->addCommandLineOptionsFromParams(input_param);
  }

  std::vector<std::string> input_filenames;
  command_line->search("input_file", input_filenames);

  auto parser = std::make_unique<Parser>(input_filenames);
  if (input_filenames.size())
    parser->parse();

  // Create an instance of the application and store it in a smart pointer for easy cleanup
  return AppFactory::createAppShared(default_app_name, argc, argv, std::move(parser));
}
}
