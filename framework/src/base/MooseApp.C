/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseApp.h"
#include "Moose.h"
#include "MooseSyntax.h"
#include "MooseInit.h"
#include "Executioner.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"

namespace Moose
{
  MooseApp * app = NULL;
}

MooseApp::MooseApp(int argc, char *argv[]) :
    _command_line(argc, argv),
    _action_warehouse(_syntax),
    _parser(_action_warehouse),
    _executioner(NULL),
    _sys_info(argc, argv),
    _enable_unused_check(WARN_UNUSED),
    _error_overridden(false)
{
  Moose::app = this;
}

MooseApp::~MooseApp()
{
  delete _executioner;
  _action_warehouse.clear();
}

void
MooseApp::init()
{
  Moose::registerObjects();
  Moose::associateSyntax(_syntax);
}

void
MooseApp::initCommandLineOptions()
{
  CommandLine::Option cli_opt;
  std::vector<std::string> syntax;

  syntax.clear();
  cli_opt.description = "Specify an input file";
  syntax.push_back("-i <input file>");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _command_line.addOption("InputFile", cli_opt);

  syntax.clear();
  cli_opt.description = "Shows the parsed input file before running the simulation";
  syntax.push_back("--show-input");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = false;
  _command_line.addOption("ShowTree", cli_opt);

  syntax.clear();
  cli_opt.description = "Displays CLI usage statement";
  syntax.push_back("-h");
  syntax.push_back("--help");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = false;
  _command_line.addOption("Help", cli_opt);

  syntax.clear();
  cli_opt.description = "Shows a dump of available input file syntax, can be filtered based on [search string]";
  syntax.push_back("--dump");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _command_line.addOption("Dump", cli_opt);

  syntax.clear();
  cli_opt.description = "Dumps input file syntax in YAML format, can be filtered based on [search string]";
  syntax.push_back("--yaml");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _command_line.addOption("YAML", cli_opt);

  syntax.clear();
  cli_opt.description = "Dumps the associated Action syntax paths ONLY";
  syntax.push_back("--syntax");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = false;
  _command_line.addOption("Syntax", cli_opt);

  syntax.clear();
  cli_opt.description = "Runs the specified number of threads (Intel TBB) per process";
  syntax.push_back("--n-threads <threads>");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _command_line.addOption("Threads", cli_opt);

  syntax.clear();
  cli_opt.description = "Warn about unused input file options";
  syntax.push_back("-w");
  syntax.push_back("--warn-unused");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _command_line.addOption("WarnUnused", cli_opt);

  syntax.clear();
  cli_opt.description = "Error when encounting unused input file options";
  syntax.push_back("-e");
  syntax.push_back("--error-unused");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _command_line.addOption("ErrorUnused", cli_opt);

  syntax.clear();
  cli_opt.description = "Error when encountering overriden or parameters supplied multipled times";
  syntax.push_back("-o");
  syntax.push_back("--error-override");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _command_line.addOption("ErrorOverride", cli_opt);

  /* This option is used in InitialRefinementAction directly - Do we need a better API? */
  syntax.clear();
  cli_opt.description = "Specify additional initial uniform refinements for automatic scaling";
  syntax.push_back("-r <refinements>");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _command_line.addOption("REFINE", cli_opt);
}

void
MooseApp::parseCommandLine()
{
  std::string input_filename;
  std::string argument;

  if (_command_line.search("ErrorUnused"))
    setCheckUnusedFlag(true);
  else if (_command_line.search("WarnUnused"))
    setCheckUnusedFlag(false);

  if (_command_line.search("ErrorOverride"))
    setErrorOverridden();

  if (_command_line.search("Help"))
  {
    _command_line.printUsage();
  }
  else if (_command_line.search("Dump", &argument))
  {
    _parser.initSyntaxFormatter(Parser::INPUT_FILE, true);
    _parser.buildFullTree(argument);
  }
  else if (_command_line.search("YAML", &argument))
  {
    _parser.initSyntaxFormatter(Parser::YAML, true);
    _parser.buildFullTree(argument);
  }
  else if (_command_line.search("Syntax"))
  {
    std::multimap<std::string, Syntax::ActionInfo> syntax = _syntax.getAssociatedActions();
    std::cout << "**START SYNTAX DATA**\n";
    for (std::multimap<std::string, Syntax::ActionInfo>::iterator it = syntax.begin(); it != syntax.end(); ++it)
    {
      std::cout << it->first << "\n";
    }
    std::cout << "**END SYNTAX DATA**\n" << std::endl;
  }
  else if (_command_line.search("InputFile", &input_filename))
  {
    _input_filename = input_filename;
    runInputFile();
  }
  else
    _command_line.printUsage();
}

void
MooseApp::runInputFile()
{
  _parser.parse(_input_filename);

  _action_warehouse.build();
  // Print the input file syntax if requested
  if (_command_line.search("ShowTree"))
  {
    _action_warehouse.printInputFile(std::cout);
  }

  _action_warehouse.executeAllActions();
  _executioner = _action_warehouse.executioner();

  // If requested, see if there are unidentified name/value pairs in the input file
  if (_command_line.search("ErrorUnused") || _enable_unused_check == ERROR_UNUSED)
  {
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, true);
  }
  else if (_command_line.search("WarnUnused") || _enable_unused_check == WARN_UNUSED)
  {
    std::vector<std::string> all_vars = _parser.getPotHandle()->get_variable_names();
    _parser.checkUnidentifiedParams(all_vars, _enable_unused_check == ERROR_UNUSED);
  }

  if (_command_line.search("ErrorOverride") || _error_overridden)
    _parser.checkOverriddenParams(true);
  else
    _parser.checkOverriddenParams(false);

  // run the simulation
  _executioner->execute();
}

void
MooseApp::setCheckUnusedFlag(bool warn_is_error)
{
  _enable_unused_check = warn_is_error ? ERROR_UNUSED : WARN_UNUSED;
}

void
MooseApp::disableCheckUnusedFlag()
{
  _enable_unused_check = OFF;
}

void
MooseApp::setErrorOverridden()
{
  _error_overridden = true;
}

void
MooseApp::run()
{
  std::cout << _sys_info.getInfo();

  initCommandLineOptions();
  _command_line.buildVarsSet();
  parseCommandLine();
}

std::string
MooseApp::getFileName(bool stripLeadingPath) const
{
  return _parser.getFileName(stripLeadingPath);
}
