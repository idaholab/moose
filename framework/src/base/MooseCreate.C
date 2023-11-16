//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseCreate.h"
#include "ParallelUniqueId.h"
#include "Parser.h"
#include "AppFactory.h"
#include "CommandLine.h"
#include "InputParameters.h"

#ifdef LIBMESH_HAVE_OPENMP
#include <omp.h>
#endif

void
MooseCreate::addParam(InputParameters & params)
{
  params.addCommandLineParam<std::vector<std::string>>(
      "input_file",
      "-i <input_files>",
      "Specify one or multiple input files. Multiple files get merged into a single simulation "
      "input.");
}

MooseCreate::MooseCreate(int argc, char * argv[])
{
  // Construct front parser
  auto front_parser = std::make_shared<Parser>();

  auto command_line = std::make_shared<CommandLine>(argc, argv);
  auto input_param = emptyInputParameters();
  addParam(input_param);
  command_line->addCommandLineOptionsFromParams(input_param);

  std::vector<std::string> input_filename;
  command_line->search("input_file", input_filename);

  if (!input_filename.empty())
    front_parser->parse(input_filename);

  // Create an instance of the application and store it in a smart pointer for easy cleanup
  _app = AppFactory::createAppShared("MooseTestApp", argc, argv, front_parser);
}
