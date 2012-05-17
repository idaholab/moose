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

#include "CommandLine.h"
#include "MooseInit.h"  /// for Moose::command_line
#include "Parser.h"

CommandLine::CommandLine(Parser &parser) :
    _parser(parser)
{
  CLIOption cli_opt;
  std::vector<std::string> syntax;

  syntax.clear();
  cli_opt.description = "Shows the parsed input file before running the simulation";
  syntax.push_back("--show-input");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = false;
  _cli_options["ShowTree"] = cli_opt;

  syntax.clear();
  cli_opt.description = "Displays CLI usage statement";
  syntax.push_back("-h");
  syntax.push_back("--help");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = false;
  _cli_options["Help"] = cli_opt;

  syntax.clear();
  cli_opt.description = "Shows a dump of available input file syntax, can be filtered based on [search string]";
  syntax.push_back("--dump");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _cli_options["Dump"] = cli_opt;

  syntax.clear();
  cli_opt.description = "Dumps input file syntax in YAML format, can be filtered based on [search string]";
  syntax.push_back("--yaml");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _cli_options["YAML"] = cli_opt;

  syntax.clear();
  cli_opt.description = "Dumps the associated Action syntax paths ONLY";
  syntax.push_back("--syntax");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = false;
  _cli_options["Syntax"] = cli_opt;

  syntax.clear();
  cli_opt.description = "Runs the specified number of threads (Intel TBB) per process";
  syntax.push_back("--n-threads <threads>");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _cli_options["Threads"] = cli_opt;

  syntax.clear();
  cli_opt.description = "Warn about unused input file options";
  syntax.push_back("-w");
  syntax.push_back("--warn-unused");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _cli_options["WarnUnused"] = cli_opt;

  syntax.clear();
  cli_opt.description = "Error when encounting unused input file options";
  syntax.push_back("-e");
  syntax.push_back("--error-unused");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _cli_options["ErrorUnused"] = cli_opt;

  /* This option is used in InitialRefinementAction directly - Do we need a better API? */
  syntax.clear();
  cli_opt.description = "Specify additional initial uniform refinements for automatic scaling";
  syntax.push_back("-r <refinements>");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  cli_opt.optional_argument = true;
  _cli_options["REFINE"] = cli_opt;
}

std::string
CommandLine::parseCommandLine()
{
  std::string input_filename;
  std::string argument;

  if (searchCommandLine("Help"))
  {
    printUsage();
    exit(0);
  }
  if (searchCommandLine("Dump", &argument))
  {
    _parser.initSyntaxFormatter(Parser::INPUT_FILE, true);

    _parser.buildFullTree(argument);
    exit(0);
  }
  if (searchCommandLine("YAML", &argument))
  {
    _parser.initSyntaxFormatter(Parser::YAML, true);

    _parser.buildFullTree(argument);
    exit(0);
  }
  if (searchCommandLine("Syntax"))
  {
    std::multimap<std::string, Syntax::ActionInfo> syntax = Moose::syntax.getAssociatedActions();
    for (std::multimap<std::string, Syntax::ActionInfo>::iterator it = syntax.begin(); it != syntax.end(); ++it)
    {
      std::cout << it->first << "\n";
    }
    exit(0);
  }

  if (Moose::command_line == NULL)
    mooseError("Command Line object is NULL! Did you create a MooseInit object?");
  if (Moose::command_line->search("-i"))
    input_filename = Moose::command_line->next(input_filename);
  else
    printUsage();

  return input_filename;
}

bool
CommandLine::searchCommandLine(const std::string &option_name, std::string *argument)
{
  std::map<std::string, CLIOption>::iterator pos;

  if (Moose::command_line == NULL)
    mooseError("Command Line object is NULL! Did you create a MooseInit object?");

  pos = _cli_options.find(option_name);
  if (pos != _cli_options.end())
    for (unsigned int i=0; i<pos->second.cli_syntax.size(); ++i)
      if (Moose::command_line->search(pos->second.cli_syntax[i]))
      {
        if (pos->second.optional_argument && argument)
          *argument = Moose::command_line->next(*argument);
        return true;
      }

  return false;
}

void
CommandLine::printUsage() const
{
  // Grab the first item out of argv
  std::string command((*Moose::command_line)[0]);
  command.substr(command.find_last_of("/\\")+1);

  std::cout << "\nUsage: " << command << " [-i <input file> --show-input | <Option>]\n\n"
            << "Options:\n" << std::left;

  for (std::map<std::string, CLIOption>::const_iterator i=_cli_options.begin(); i != _cli_options.end(); ++i)
  {
    std::stringstream oss;
    for (unsigned int j=0; j<i->second.cli_syntax.size(); ++j)
    {
      if (j) oss << " | ";
      oss << i->second.cli_syntax[j];
    }
    std::cout << "  " << std::setw(50) << oss.str() << i->second.description << "\n";
  }

  std::cout << "\nSolver Options:\n"
            << "  See solver manual for details (Petsc or Trilinos)\n";
  exit(0);
}

void
CommandLine::buildCommandLineVarsSet()
{
  if (Moose::command_line == NULL)
    return;

  for(const char* var; (var = Moose::command_line->next_nominus()) != NULL; )
  {
    std::vector<std::string> name_value_pairs;
    Parser::tokenize(var, name_value_pairs, 0, "=");
    _command_line_vars.insert(name_value_pairs[0]);
  }
}

bool
CommandLine::isVariableOnCommandLine(const std::string &name) const
{
  return _command_line_vars.find(name) != _command_line_vars.end();
}
