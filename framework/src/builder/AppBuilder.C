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
#include "MooseMain.h"

namespace Moose
{

AppBuilder::AppBuilder(std::shared_ptr<Parser> parser, const bool catch_parse_errors)
  : BuilderBase(parser), _catch_parse_errors(catch_parse_errors)
{
}

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

  // Determine the overriden command line type, if any
  {
    CommandLine type_command_line(argc, argv);
    type_command_line.parse();

    // Get the hit command line parameters
    auto cli_root = parseCLIArgs(type_command_line);

    // Don't allow Application/app= on command line
    if (cli_root->find("Application/app"))
      mooseError("The command-line input option Application/app is not supported. Please use "
                 "Application/type= or --type instead.");
    // Check for Application/type= on command line
    const auto cli_hit_type_node = cli_root->find("Application/type");
    if (cli_hit_type_node)
      type = cli_hit_type_node->param<std::string>();

    // Search for --app and --type on command line
    auto params = emptyInputParameters();
    MooseApp::addTypeParam(params);
    type_command_line.populateCommandLineParams(params);
    const auto has_cli_app = params.isParamSetByUser("app");

    // Check for --type on command line
    if (params.isParamSetByUser("type"))
    {
      if (cli_hit_type_node)
        mooseError("You cannot specify --type and Application/type together on the command line");
      if (has_cli_app)
        mooseError("You cannot specify the command-line options --type and --app together.");
      type = params.get<std::string>("type");
    }
    // Check for --app on command line
    if (has_cli_app)
    {
      type = params.get<std::string>("app");
      mooseDeprecated("The specified command line option '--app ",
                      type,
                      "' is deprecated and will be removed in a future release.\n\nPlease use "
                      "'--type' via command line or 'Application/type' in input.");
    }
  }

  if (!AppFactory::instance().isRegistered(type))
    mooseError("The application type '", type, "' is not registered.");

  // Setup the command line
  auto command_line = std::make_shared<CommandLine>(argc, argv);
  command_line->parse();

  auto params = AppFactory::instance().getValidParams(type);
  params.set<std::string>("_type") = type;
  params.set<int>("_argc") = argc;
  params.set<char **>("_argv") = argv;
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
  mooseAssert(params.get<std::string>("_type").size(), "_type param is not set");

  params.set<std::string>("_app_name") = name;
  params.set<std::shared_ptr<Parser>>("_parser") = _parser;

  // Setup communicator
  auto comm = std::make_shared<Parallel::Communicator>(comm_world_in);
  params.set<std::shared_ptr<Parallel::Communicator>>("_comm") = comm;

  // Setup a shared state from this process that needs to be used by the Builder later
  auto state = std::make_shared<AppBuilder::State>();
  params.set<std::shared_ptr<AppBuilder::State>>("_app_builder_state") = state;

  auto command_line = params.get<std::shared_ptr<CommandLine>>("_command_line");
  mooseAssert(command_line, "_command_line not set");
  // Populate the command line arguments
  command_line->populateCommandLineParams(params);
  // Populate the command line hit arguments
  state->cli_root = mergeCLIArgs(*command_line);

  // Make sure that we don't have a cli switch and a hit param for the same thing
  for (const auto & name_value_pair : params)
  {
    const auto & name = name_value_pair.first;
    if (const auto cl_metadata_ptr = params.queryCommandLineMetadata(name))
      if (cl_metadata_ptr->set_by_switch && state->cli_root->find("Application/" + name))
        mooseError("The command-line option '",
                   *cl_metadata_ptr->set_by_switch,
                   "' and the command-line parameter 'Application/",
                   name,
                   "' apply to the same value and cannot be set together.");
  }

  // Make sure that Application/input_file= is not used
  if (state->cli_root->find("Application/input_file"))
    mooseError("The command-line parameter 'Application/input_file=' cannot be used. Use the "
               "command-line option '-i <input file(s)> instead.");

  // Do the initial walk, which does expansion and as much early error checking as we
  // can do on the _entire_ input, not just [Application]
  initialWalk();

  // Add input parameters from Application/* in input
  extractParams("Application", params, nullptr, {});

  // Store the extracted variables so that they can be used in unused variable checking
  // later on in the Builder when we have all the things extracted
  state->extracted_vars = _extracted_vars;

  // Check required parameters
  params.checkParams("");
}

void
AppBuilder::initialWalk()
{
  std::string error_message;
  const auto add_errors = [this, &error_message](auto & errors)
  {
    for (auto & error : errors)
    {
      if (!_catch_parse_errors || error.node().fullpath().rfind("Application/", 0) == 0)
        error_message += error.fullMessage();
      else
        _parse_errors.emplace_back(error);
    }
  };

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
  add_errors(exw.errors);

  // Now that we have accumulated the command line hit parameters, we can make this check
  BadActiveWalker bw;
  root().walk(&bw, hit::NodeType::Section);
  add_errors(bw.errors);

  // Print parse errors if we have any to report
  if (error_message.size())
    mooseError(error_message);
}

}
