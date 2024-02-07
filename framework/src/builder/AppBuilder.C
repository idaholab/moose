//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AppBuilder.h"

#include "CommandLine.h"
#include "MooseApp.h"
#include "AppFactory.h"

namespace Moose
{

AppBuilder::AppBuilder(std::shared_ptr<Parser> parser) : BuilderBase(parser) {}

InputParameters
AppBuilder::buildParams(const std::string & default_type,
                        const std::string & name,
                        int argc,
                        char ** argv,
                        MPI_Comm comm_world_in)
{
  _extracted_vars.clear();

  // Start with either Application/type= in input, or the default app type
  std::string type = parser().getInputAppType() ? *parser().getInputAppType() : default_type;

  // Search for application type via command line
  {
    CommandLine type_command_line(argc, argv);
    auto params = emptyInputParameters();
    MooseApp::addTypeParam(params);
    type_command_line.addCommandLineOptionsFromParams(params);

    // Search for Application/[type,app]= on command line
    auto cli_root = parseCLIArgs(name, type_command_line);
    if (cli_root->find("Application/app"))
      mooseError("The command-line input option Application/app is not supported. Please use "
                 "Application/type= or --type instead.");
    if (const auto node = cli_root->find("Application/type"))
      type = node->param<std::string>();

    // Search for --[type,app] on command line
    const bool has_cli_type = type_command_line.search("type", type);
    if (type_command_line.search("app", type))
    {
      if (has_cli_type)
        mooseError("You cannot specify the command-line options --type and --app together.");
      else
        mooseDeprecated("Please use Application/type= or --type <AppName> via command line "
                        "to specify application type; '--app <AppName>' is deprecated and will be "
                        "removed in a future release.");
    }
  }

  auto params = AppFactory::instance().getValidParams(type);
  params.set<int>("_argc") = argc;
  params.set<char **>("_argv") = argv;

  // Setup the command line
  auto command_line = std::make_shared<CommandLine>(argc, argv);
  params.set<std::shared_ptr<CommandLine>>("_command_line") = command_line;

  // Setup the rest of the state, extract the Application/* parameters
  buildParamsFromCommandLine(name, params, comm_world_in);

  return params;
}

void
AppBuilder::buildParamsFromCommandLine(const std::string & name,
                                       InputParameters & params,
                                       MPI_Comm comm_world_in)
{
  params.set<std::string>("_app_name") = name;
  params.set<std::shared_ptr<Parser>>("_parser") = _parser;

  // Setup communicator
  auto comm = std::make_shared<Parallel::Communicator>(comm_world_in);
  params.set<std::shared_ptr<Parallel::Communicator>>("_comm") = comm;

  // Setup a shared state from this process that needs to be used by the Builder later
  auto state = std::make_shared<AppBuilder::State>();
  params.set<std::shared_ptr<AppBuilder::State>>("_app_builder_state") = state;

  // Get the command line
  auto command_line = params.get<std::shared_ptr<CommandLine>>("_command_line");
  mooseAssert(command_line, "_command_line not set");

  // Merge in CLI arguments
  // We keep track of this so that we can do error checking on it later in the Builder
  // once all of the input has been processed
  state->cli_root = mergeCLIArgs(name, *command_line);

  // Do the initial walk, which does expansion and as much early error checking as we
  // can do on the _entire_ input, not just [Application]
  initialWalk();

  // Add input parameters from Application/* in input
  extractParams("Application", params, nullptr, {});

  // Store the extracted variables so that they can be used in unused variable checking
  // later on in the Builder when we have all the things extracted
  state->extracted_vars = _extracted_vars;

  // Fill the command line arguments (non hit params) from command line
  command_line->addCommandLineOptionsFromParams(params);
  command_line->populateInputParams(params);

  // Check required parameters
  params.checkParams("");
}

void
AppBuilder::initialWalk()
{
  // expand ${bla} parameter values and mark/include variables used in expansions as "used".  This
  // MUST occur before parameter extraction - otherwise parameters will get wrong values.
  hit::RawEvaler raw;
  hit::EnvEvaler env;
  hit::ReplaceEvaler repl;
  FuncParseEvaler fparse_ev;
  UnitsConversionEvaler units_ev;
  hit::BraceExpander exw;
  exw.registerEvaler("raw", raw);
  exw.registerEvaler("env", env);
  exw.registerEvaler("fparse", fparse_ev);
  exw.registerEvaler("replace", repl);
  exw.registerEvaler("units", units_ev);
  root().walk(&exw);
  for (auto & var : exw.used)
    _extracted_vars.insert(var);
  for (auto & msg : exw.errors)
    _errmsg += msg + "\n";

  // Now that we have accumulated the command line hit parameters, we can make this check
  BadActiveWalker bw;
  root().walk(&bw, hit::NodeType::Section);
  for (auto & msg : bw.errors)
    _errmsg += msg + "\n";

  // Print parse errors related to brace expansion early
  if (_errmsg.size() > 0)
    mooseError(_errmsg);
}

}
